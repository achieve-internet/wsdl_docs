<?php

namespace Drupal\Tests\wsdl_docs\Kernel;

use Drupal;
use Drupal\Core\Entity\EntityStorageException;
use Drupal\KernelTests\Core\Entity\EntityKernelTestBase;
use Drupal\node\Entity\Node;

/**
 * Class SoapClientManagerTest.
 *
 * @group wsdl_docs
 */
class SoapClientManagerTest extends EntityKernelTestBase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = [
    'node',
    'field',
    'options',
    'text',
    'file',
    'path',
    'link',
    'menu_ui',
    'wsdl_docs',
    'views',
  ];

  /**
   * Profile to enable.
   *
   * @var string
   */
  protected $profile = 'standard';


  /**
   * Admin user.
   *
   * @var \Drupal\Core\Session\AccountInterface
   */
  protected $adminUser;

  /**
   * Soap Client manager.
   *
   * @var \Drupal\wsdl_docs\Services\SoapClientManager
   */
  protected $soapManagerService;

  /**
   * Amazon test WSDL url.
   *
   * @var string
   */
  protected $soapTestUrl;

  /**
   * Set up initial prerequisites.
   */
  public function setUp() {
    parent::setUp();
    $this->installConfig(['node', 'wsdl_docs']);
    try {
      $this->setUpCurrentUser();
    }
    catch (EntityStorageException $e) {

    }
    $this->soapManagerService = Drupal::service('wsdl_docs.soap_client_manager');
    $this->soapTestUrl = 'http://s3.amazonaws.com/ec2-downloads/ec2.wsdl';
  }

  /**
   * Test Soap Service node validations.
   */
  public function testViolations() {
    $node = Node::create(
      [
        'title' => [['value' => '']],
        'body' => [['value' => '']],
        'field_wsdl_docs_source' => [['uri' => '']],
        'type' => 'soap_service',
      ]
    );
    $violations = $node->validate();
    $this->assertEqual(count($violations), 2);
    $this->assertEqual($violations[0]->getPropertyPath(), 'title');
    $this->assertEqual($violations[0]->getMessage(), 'This value should not be null.');
    $this->assertEqual($violations[1]->getPropertyPath(), 'field_wsdl_docs_source');
    $this->assertEqual($violations[1]->getMessage(), 'This value should not be null.');
  }

  /**
   * Test creation of WSDL Docs operations from Soap Service.
   */
  public function testSoapClientManager() {
    // Creates a soap service node with test amazon wsdl.
    $node = Node::create(
      [
        'title' => [['value' => 'Amazon SOAP Test']],
        'body' => [['value' => 'A test SOAP Service using a large Amazon WSDL.']],
        'field_wsdl_docs_source' => [['uri' => $this->soapTestUrl]],
        'type' => 'soap_service',
        'status' => 1,
      ]
    );
    try {
      $node->save();
    }
    catch (EntityStorageException $e) {
      $this->assertNotEmpty($node->id(), 'Service node was created successfully with label' . $node->label());
    }
    $this->assertNotEmpty($node->id(), 'Service node was created successfully with nid ' . $node->id());
    // Code below verifies wsdl_doc_operation nodes.
    $client = $this->soapManagerService->loadUrl($this->soapTestUrl);
    $this->assertNotEmpty($client, 'Created a SoapClient for amazon test SOAP endpoint url.');

    $data_types = $this->soapManagerService->parseSoapTypes($client->__getTypes());
    $this->assertNotEmpty($data_types, 'Successfully loaded a list of SOAP types.');

    // Load WSDL file.
    $wsdl = file_get_contents($this->soapTestUrl);
    $this->assertNotEmpty($wsdl, 'Get contents of WSDL file ' . $this->soapTestUrl . '.');

    $domDocument = $this->soapManagerService->getDomdocument($wsdl);
    $this->assertNotEmpty($domDocument, 'Loads DOMDocument object if XML validates, FALSE if invalid');

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
        $message = $this->soapManagerService->removeMethodNamespace($message);
        $outputs_messages[$message] = $operation_name;
        $_outputs_messages[$operation_name][] = $message;

        // Repeat for input.
        $message = $operation->getElementsByTagName('input')[0]->getAttribute('message');
        // Remove "tns:" namespace so we can parse the element.
        $message = $this->soapManagerService->removeMethodNamespace($message);
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
          $element = $this->soapManagerService->removeMethodNamespace($element);
          $messages_elements[$element][] = $message_name;
          $_messages_elements[$message_name] = [
            'name' => $part_name,
            'element' => $element,
          ];
        }
        elseif ($part->hasAttribute('type')) {
          $type = $part->getAttribute('type');
          // Remove "tns:" namespace so we can parse the element.
          $type = $this->soapManagerService->removeMethodNamespace($type);
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
              $element_type = $this->soapManagerService->removeMethodNamespace($element_type);
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
        $outputs[$name] = $this->soapManagerService->renderOperation($name, $_outputs_messages, $_messages_elements, $_elements_types, $_types_properties, $data_types);
        $inputs[$name] = $this->soapManagerService->renderOperation($name, $_inputs_messages, $_messages_elements, $_elements_types, $_types_properties, $data_types);
      }
    }

    // List of new operations.
    $operations = $this->soapManagerService->parseSoapOperations($client->__getFunctions());
    // Find number of WSDL Docs operation nodes.
    $current_operation = $this->entityTypeManager->getStorage('node')
      ->loadByProperties([
        'type' => 'wsdl_docs_operation',
        'field_wsdl_docs_soap_ref' => $node->id(),
      ]);
    // Match of no of operations with wsdl docs operation
    // nodes referencing soap service node.
    $this->assertEqual(count($operations), count($current_operation), 'Number of operations should match with WSDL Docs operation nodes.');

    // New operations left.
    foreach ($operations as $name => $operation) {
      $current_operation = $this->entityTypeManager->getStorage('node')
        ->loadByProperties([
          'type' => 'wsdl_docs_operation',
          'field_wsdl_docs_soap_ref' => $node->id(),
          'title' => $operation['label'],
        ]);
      // Check if no matching wsdl docs operation node present.
      $this->assertNotEmpty($current_operation, 'Load wsdl operation node with matching label ' . $operation['label'] . ' and Soap service node id' . $node->id() . '.');
      // WSDL Docs operations node.
      $current_operation = reset($current_operation);

      // Compare.
      $docstyle = isset($styles[$operation['label']]) ? $styles[$operation['label']] : '';
      $this->assertEqual($current_operation->field_wsdl_docs_style->value, $docstyle, 'Input field "field_wsdl_docs_style" value should match for WSDL Docs Operation node with ' . $current_operation->label() . '.');

      $documentation = isset($documentations[$operation['label']]) ? $documentations[$operation['label']] : '';
      $this->assertEqual($documentation, $current_operation->field_wsdl_docs_documentation->value, 'Input field "field_wsdl_docs_documentation" value should match for WSDL Docs Operation node with ' . $current_operation->label() . '.');

      $output = isset($outputs[$operation['label']]) ? $outputs[$operation['label']] : '';
      $this->assertEqual($current_operation->field_wsdl_docs_output->value, $output, 'Input field "field_wsdl_docs_output" value should match for WSDL Docs Operation node with ' . $current_operation->label() . '.');

      $input = isset($inputs[$operation['label']]) ? $inputs[$operation['label']] : '';
      $this->assertEqual($current_operation->field_wsdl_docs_input->value, $input, 'Input field "field_wsdl_docs_input" value should match for WSDL Docs Operation node with ' . $current_operation->label() . '.');
    }
  }

}
