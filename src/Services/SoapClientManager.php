<?php

namespace Drupal\wsdl_docs\Services;

use DOMDocument;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\File\FileSystemInterface;
use Drupal\Core\Logger\LoggerChannelFactoryInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\node\Entity\Node;
use Drupal\node\NodeInterface;
use SoapClient;
use Throwable;

/**
 * Class SoapClientManager.
 *
 * Import and manage WSDL documents.
 *
 * @package Drupal\wsdl_docs\Services
 */
class SoapClientManager {

  /**
   * The logger factory service.
   *
   * @var \Drupal\Core\Logger\LoggerChannelFactoryInterface
   */
  protected $loggerFactory;

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * The file system service.
   *
   * @var \Drupal\Core\File\FileSystemInterface
   */
  protected $fileSystem;

  /**
   * Current user.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $currentUser;

  /**
   * ProductManager constructor.
   *
   * @param \Drupal\Core\Logger\LoggerChannelFactoryInterface $logger_factory
   *   Logger factory service.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   Entity type manager service.
   * @param \Drupal\Core\File\FileSystemInterface $file_system
   *   File system service.
   * @param \Drupal\Core\Session\AccountInterface $current_user
   *   The current user.
   */
  public function __construct(LoggerChannelFactoryInterface $logger_factory, EntityTypeManagerInterface $entity_type_manager, FileSystemInterface $file_system, AccountInterface $current_user) {
    $this->loggerFactory = $logger_factory;
    $this->entityTypeManager = $entity_type_manager;
    $this->fileSystem = $file_system;
    $this->currentUser = $current_user;
  }

  /**
   * Returns a SoapClient given a soap_service node.
   *
   * @param \Drupal\node\NodeInterface $node
   *   Soap service node.
   *
   * @return null|SoapClient
   *   SoapClient, or NULL if error.
   */
  public function loadSoapClientFromNode(NodeInterface $node) {
    $uri = $this->getUrl($node);
    if (empty($uri)) {
      return NULL;
    }
    return $this->loadUrl($uri);
  }

  /**
   * Returns a SOAP endpoint URI given a soap_service node.
   *
   * @param \Drupal\node\NodeInterface $node
   *   Soap service node.
   *
   * @return null|string
   *   URI string, or NULL if error.
   */
  public function getUrl(NodeInterface $node) {
    $uri = $node->get('field_wsdl_docs_source')->getValue();
    if (empty($uri)) {
      return NULL;
    }
    return $uri[0]['uri'];
  }

  /**
   * Create a SoapClient given a SOAP endpoint url.
   *
   * @param string $uri
   *   URI of the WSDL file.
   *
   * @return \SoapClient|null
   *   SoapClient service loaded with the provided URI, or null if error.
   */
  public function loadUrl($uri) {
    try {
      $client = new SoapClient($uri, ["trace" => 1, "exception" => 0]);
    }
    catch (Throwable $t) {
      $this->loggerFactory->get('wsdl_docs')
        ->warning('Problem loading SoapClient with uri @uri, message: @message', [
          '@uri' => $uri,
          '@message' => $t->getMessage(),
        ]);
      return NULL;
    }
    return $client;
  }

  /**
   * Parse a SOAP Service WSDL into Operations.
   *
   * Function called during create/update of soap_service nodes to process
   * linked WSDL file into wsdl_docs_operation nodes.
   *
   * @param \Drupal\node\NodeInterface $node
   *   Soap Service node.
   *
   * @return bool
   *   Success status of operation.
   */
  public function saveSoapNode(NodeInterface $node) {
    if (!$node->getType() == 'soap_service') {
      return FALSE;
    }
    $this->updateOperations($node);
  }

  /**
   * Create/update/delete nodes derived from WSDL.
   *
   * @param \Drupal\node\NodeInterface $node
   *   SOAP Service node with link to WSDL.
   * @param \DOMDocument|null $domDocument
   *   DOMDocument of the WSDL.
   *
   * @return null
   *   Returns early on error.
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  public function updateOperations(NodeInterface $node, DOMDocument $domDocument = NULL) {
    $user = $this->currentUser;
    $user = $this->entityTypeManager->getStorage('user')->load($user->id());
    $uri = $this->getUrl($node);
    if (empty($uri)) {
      return NULL;
    }
    $client = $this->loadUrl($uri);
    $data_types = $this->parseSoapTypes($client->__getTypes());

    if (!$domDocument) {
      $wsdl = file_get_contents($uri);
      $domDocument = $this->getDomdocument($wsdl);
      if (!$domDocument) {
        // XML didn't validate so stop here.
        return NULL;
      }
    }

    $documentations = $styles = $outputs = $outputs_messages = $inputs = $inputs_messages = $messages_elements = $elements_types = [];

    $portTypes = $domDocument->getElementsByTagName('portType');
    foreach ($portTypes as $portType) {
      $operations = $portType->getElementsByTagName('operation');
      foreach ($operations as $operation) {
        $operation_name = $operation->getAttribute('name');
        // Parse documentation element.
        if (!isset($documentations[$operation_name])) {
          $documentation = $operation->getElementsByTagName('documentation');
          if ($documentation->length > 0) {
            $documentations[$operation_name] = $documentation[0]->nodeValue;
          }
        }

        // For input/output (step 1):
        // parsing correct input/output like WSDL viewer is multi step process.
        $message = $operation->getElementsByTagName('output')[0]->getAttribute('message');
        // Remove "tns:" namespace so we can parse the element.
        $message = $this->removeMethodNamespace($message);
        $outputs_messages[$message] = $operation_name;
        $_outputs_messages[$operation_name][] = $message;

        // Repeat for input.
        $message = $operation->getElementsByTagName('input')[0]->getAttribute('message');
        // Remove "tns:" namespace so we can parse the element.
        $message = $this->removeMethodNamespace($message);
        $inputs_messages[$message] = $operation_name;
        $_inputs_messages[$operation_name][] = $message;
      }
    }

    // For input/output (step 2)
    $messages = $domDocument->getElementsByTagName('message');
    foreach ($messages as $message) {
      $message_name = $message->getAttribute('name');
      if (isset($outputs_messages[$message_name]) || isset($inputs_messages[$message_name])) {
        $part = $message->getElementsByTagName('part')[0];
        $part_name = $part->getAttribute('name');
        if ($part->hasAttribute('element')) {
          $element = $part->getAttribute('element');
          // Remove "tns:".
          $element = $this->removeMethodNamespace($element);
          $messages_elements[$element][] = $message_name;
          $_messages_elements[$message_name] = [
            'name' => $part_name,
            'element' => $element,
          ];
        }
        elseif ($part->hasAttribute('type')) {
          $type = $part->getAttribute('type');
          // Remove "tns:" namespace so we can parse the element.
          $type = $this->removeMethodNamespace($type);
          $messages_elements[$type][] = $message_name;
          $_messages_elements[$message_name] = [
            'name' => $part_name,
            'type' => $type,
          ];
        }
      }
    }

    // Parse input/output (step 3)
    $schemas = $domDocument->getElementsByTagName('types')[0]->getElementsByTagName('schema');
    foreach ($schemas as $schema) {
      $elements = $schema->childNodes;
      foreach ($elements as $element) {
        if ($element->localName == 'element') {
          $element_name = $element->getAttribute('name');
          $element2_name = $element_name;
          if (isset($messages_elements[$element_name])) {
            // Parse complexType element.
            if (!$element->hasAttribute('type')) {
              $element = $element->getElementsByTagName('element')[0];
              $element2_name = $element->getAttribute('name');
            }
            if ($element->hasAttribute('type')) {
              $element_type = $element->getAttribute('type');
              // Remove "tns:".
              $element_type = $this->removeMethodNamespace($element_type);
              $elements_types[$element_type][] = $element_name;
              $_elements_types[$element_name] = [
                'name' => $element2_name,
                'type' => $element_type,
              ];
            }
          }
        }
      }
    }

    // Parse input/output (step 4)
    foreach ($elements_types as $type => $element) {
      $_types_properties[$type] = $data_types[$type]['property info'];
    }
    $operations = $domDocument->getElementsByTagName('binding')[0]->childNodes;
    foreach ($operations as $operation) {
      if ($operation->localName == 'operation') {
        $name = $operation->getAttribute('name');
        // Parse style element.
        $operation2 = $operation->getElementsByTagName('operation')[0];
        if ($operation2->hasAttribute('style')) {
          $styles[$name] = $operation2->getAttribute('style');
        }
        // Parse body element.
        $outputs[$name] = $this->renderOperation($name, $_outputs_messages, $_messages_elements, $_elements_types, $_types_properties);
        $inputs[$name] = $this->renderOperation($name, $_inputs_messages, $_messages_elements, $_elements_types, $_types_properties);
      }
    }

    // List of new operations.
    $operations = $this->parseSoapOperations($client->__getFunctions());
    // Get old operations for this service.
    $current_operations = $this->entityTypeManager->getStorage('node')
      ->loadByProperties([
        'type' => 'wsdl_docs_operation',
        'field_wsdl_docs_soap_ref' => $node->id(),
      ]);
    foreach ($current_operations as $current_operation) {
      // If old operation exists in new operations list.
      if (isset($operations[$current_operation->label()])) {
        // Get new operation data.
        $operation = $operations[$current_operation->label()];
        // Set new documentation, style and output data.
        $current_operation->set('field_wsdl_docs_style', isset($styles[$operation['label']]) ? $styles[$operation['label']] : '');
        $current_operation->set('field_wsdl_docs_documentation', isset($documentations[$operation['label']]) ? [
          'value' => $documentations[$operation['label']],
          'format' => 'full_html',
        ] : '');
        $current_operation->set('field_wsdl_docs_output', isset($outputs[$operation['label']]) ? [
          'value' => $outputs[$operation['label']],
          'format' => 'full_html',
        ] : '');
        $current_operation->set('field_wsdl_docs_input', isset($inputs[$operation['label']]) ? [
          'value' => $inputs[$operation['label']],
          'format' => 'full_html',
        ] : '');
        // Update old operation node.
        $current_operation->save();
        // Unset old operation from new operations list.
        unset($operations[$current_operation->label()]);
      }
      // Else if old operation does not exist in new operations list.
      else {
        $current_operation->delete();
      }
    }

    // New operations left.
    foreach ($operations as $name => $operation) {
      // Create operation node.
      $new_operation = Node::create([
        'type' => 'wsdl_docs_operation',
        'title' => $operation['label'],
        'status' => TRUE,
        'field_wsdl_docs_soap_ref' => [
          'target_id' => $node->id(),
        ],
        'field_wsdl_docs_style' => [
          'value' => isset($styles[$operation['label']]) ? $styles[$operation['label']] : '',
        ],
        'field_wsdl_docs_documentation' => [
          'value' => isset($documentations[$operation['label']]) ? $documentations[$operation['label']] : '',
          'format' => 'full_html',
        ],
        'field_wsdl_docs_output' => [
          'value' => $outputs[$operation['label']] ?: '',
          'format' => 'full_html',
        ],
        'field_wsdl_docs_input' => [
          'value' => $inputs[$operation['label']] ?: '',
          'format' => 'full_html',
        ],
      ]);
      $new_operation->setOwner($user);
      $new_operation->save();
    }
  }

  /**
   * Parse Types.
   *
   * Convert metadata about data types provided by a SOAPClient into a
   * compatible data type array.
   *
   * @param array $types
   *   The array containing the struct strings.
   *
   * @return array
   *   A data type array with property information.
   */
  public function parseSoapTypes(array $types) {
    $wsclient_types = [];
    foreach ($types as $type_string) {
      if (strpos($type_string, 'struct') === 0) {
        $parts = explode('{', $type_string);
        // Cut off struct and whitespaces from type name.
        $type_name = trim(substr($parts[0], 6));
        $wsclient_types[$type_name] = ['label' => $type_name];
        $property_string = $parts[1];
        // Cut off trailing '}'.
        $property_string = substr($property_string, 0, -1);
        $properties = explode(';', $property_string);
        // Remove last empty element.
        array_pop($properties);
        // Initialize empty property information.
        $wsclient_types[$type_name]['property info'] = [];
        foreach ($properties as $property_string) {
          // Cut off white spaces.
          $property_string = trim($property_string);
          $parts = explode(' ', $property_string);
          $property_type = $parts[0];
          $property_name = $parts[1];
          $wsclient_types[$type_name]['property info'][$property_name] = [
            'type' => $this->soapMapper($property_type),
          ];
        }
      }
    }
    return $wsclient_types;
  }

  /**
   * Maps data type names from SOAPClient to wsclient/rules internals.
   *
   * @param string $type
   *   Type string.
   *
   * @return bool|string
   *   Mapped type.
   */
  private function soapMapper($type) {
    $primitive_types = [
      'string',
      'int',
      'long',
      'float',
      'boolean',
      'double',
      'short',
      'decimal',
    ];

    if (in_array($type, $primitive_types)) {
      switch ($type) {
        case 'double':
        case 'float':
          return 'decimal';

        case 'int':
        case 'long':
        case 'short':
          return 'integer';

        case 'string':
          return 'text';
      }
    }
    // Check for list types.
    if (strpos($type, 'ArrayOf') === 0) {
      $type = substr($type, 7);
      $primitive = strtolower($type);
      if (in_array($primitive, $primitive_types)) {
        return 'list<' . $primitive . '>';
      }
      return 'list<' . $type . '>';
    }
    // Otherwise return the type as is.
    return $type;
  }

  /**
   * Validate XML and create DOMDocument object if valid.
   *
   * @param string $xml
   *   XML loaded from WSDL/XSD file.
   *
   * @return \DOMDocument|false
   *   returns loaded DOMDocument object if XML validates, FALSE if invalid.
   */
  public function getDomdocument($xml) {
    $domDocument = new DOMDocument();
    // Validate XML when loading.
    libxml_use_internal_errors(TRUE);
    $domDocument->loadXML($xml);
    $errors = libxml_get_errors();
    if (!empty($errors)) {
      $this->loggerFactory->get('wsdl_docs')
        ->warning('Problem parsing DOM document, errors: @errors', [
          '@errors' => $errors,
        ]);
      return FALSE;
    }
    libxml_clear_errors();
    return $domDocument;
  }

  /**
   * Removes namespace from WSDL method name.
   *
   * @param string $str
   *   String to de-namespace.
   *
   * @return string
   *   String with namespace removed.
   */
  public function removeMethodNamespace($str) {
    $arr = explode(':', $str, 2);
    return isset($arr[1]) ? $arr[1] : $arr[0];
  }

  /**
   * Generate HTML output for SOAP operation.
   *
   * @param string $operation_name
   *   Name of service operation.
   * @param array $_outputs_messages
   *   List of messages in output tags.
   * @param array $_messages_elements
   *   List of elements in message tags.
   * @param array $_elements_types
   *   List of types in element tags.
   * @param array $_types_properties
   *   List of element properties in data types tags.
   *
   * @return string
   *   HTML output.
   */
  public function renderOperation($operation_name, array &$_outputs_messages, array &$_messages_elements, array &$_elements_types, array &$_types_properties) {
    $messages = $_outputs_messages[$operation_name];
    // Parse port name.
    $message = $messages[0];
    $part_name = $_messages_elements[$message]['name'];
    $text = '';
    if (isset($_messages_elements[$message]['element'])) {
      $element = $_messages_elements[$message]['element'];
      $element_name = $_elements_types[$element]['name'];
      $element_type = $_elements_types[$element]['type'];
      $properties = $_types_properties[$element_type];
      $text .= $part_name . ' type ' . $element . '<br>';
      $text .= '<ul><li>' . $element_name . ' type ' . $element_type . '<ul>';
      foreach ($properties as $property_name => $property) {
        $text .= '<li>' . $property_name . ' - type ' . $property['type'] . '</li>';
      }
      $text .= '</ul></li></ul>';
    }
    elseif (isset($_messages_elements[$message]['type'])) {
      $type = $_messages_elements[$message]['type'];
      $properties = $_types_properties[$type];
      $text .= $part_name . ' type ' . $type . '<br>';
      $text .= '<ul>';
      foreach ($properties as $property_name => $property) {
        $text .= '<li>' . $property_name . ' - type ' . $property['type'] . '</li>';
      }
      $text .= '</ul>';
    }
    return $text;
  }

  /**
   * Parse Operations.
   *
   * Convert metadata about operations provided by a SOAPClient into a
   * compatible operations array.
   *
   * @param array $operations
   *   The array containing the operation signature strings.
   *
   * @return array
   *   An operations array with parameter information.
   */
  public function parseSoapOperations(array $operations) {
    $wsclient_operations = [];
    foreach ($operations as $operation) {
      $parts = explode(' ', $operation);
      $return_type = $this->soapMapper($parts[0]);
      $name_parts = explode('(', $parts[1]);
      $op_name = $name_parts[0];
      $wsclient_operations[$op_name] = [
        'label' => $op_name,
        'result' => ['type' => $return_type, 'label' => $return_type],
      ];
      $parts = explode('(', $operation);
      // Cut off trailing ')'.
      $param_string = substr($parts[1], 0, -1);
      if ($param_string) {
        $parameters = explode(',', $param_string);
        foreach ($parameters as $parameter) {
          $parameter = trim($parameter);
          $parts = explode(' ', $parameter);
          $param_type = $parts[0];
          // Remove leading '$' from parameter name.
          $param_name = substr($parts[1], 1);
          $wsclient_operations[$op_name]['parameter'][$param_name] = [
            'type' => $this->soapMapper($param_type),
          ];
        }
      }
    }
    return $wsclient_operations;
  }

}
