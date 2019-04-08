<?php
/**
 * @file
 * Utility functions for displaying and testing SOAP services.
 */

/**
 * FAPI form used to display the options and the results of calling a web
 * service.
 */
function wsdl_docs_operation_invoke($form, $form_state, $service, $operation) {
  $strings = array(
    '!service_label' => $service->label,
    '!operation_label' => $operation['label'],
  );
  drupal_set_title(t("!service_label : !operation_label()", $strings));
  $form  = array();

  $form['header-title'] = array(
    '#markup' => '<h2>Operation: ' . $operation['label'] . '</h2><br>',
  );

  $form['parameters'] = array(
    '#title' => 'Parameters',
    '#type' => 'container',
    '#tree' => TRUE,
    '#value' => t("Enter the parameters to the !operation_label service here. It's up to you to get the data types right. No type validation is done at this end, as it's a debugger to let you throw various errors at the web service and see how it responds.", $strings),
  );

  // Custom css for form parameters.
  $form['parameters']['#attached']['css'][] = drupal_get_path('module', 'wsdl_docs') . '/ui/ui.css';

  // Deal with complex types.
  // Each complex type may require its own mini-form for data entry,
  // and these may be nested.
  $datatypes = $service->datatypes;
  foreach ($operation['parameter'] as $param => $info) {
    $form['parameters'][$param] = wsdl_docs_request_data($param, $info['type'], @$info['description'], @$form_state['values']['parameters'], $datatypes, @$info['required']);
  }

  $form['execute'] = array(
    '#type' => 'submit',
    '#value' => 'Execute Request',
    '#ajax' => array(
      'event' => 'click',
      'callback' => 'wsdl_docs_prepare_request_callback',
      'wrapper' => 'edit-transaction',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );

  // Add some markup here.
  $form['spacer'] = array(
    '#markup' => '<br/><br/>',
  );

  // Set up result panes. These are the form fields on the operation nodes for the request. Content for these usually gets filled in via AJAX.

  $element_id = 'edit-transaction';
  $form['transaction'] = array(
    '#type' => 'fieldset',
    '#title' => 'Transaction',
    '#attributes' => array('id' => $element_id),
  );

  // Add tabs.
  $form['transaction']['#attached']['library'][] = array('system', 'ui.tabs');
  $form['transaction']['#attached']['js'][] = array(
    'data' => drupal_get_path('module', 'wsdl_docs') . '/wsdl_docs.js',
    'type' => 'file',
  );
  $form['transaction']['#attached']['js'][] = array(
    'data' => array('wsdl_docs' => array('element_id' => $element_id)),
    'type' => 'setting',
  );

  $form['transaction']['tabs'] = array(
    '#theme' => 'item_list',
    '#items' => array(
      '<a href="#tabs-request">Request</a>',
      '<a href="#tabs-response">Response</a>',
    ),
  );

  $form['transaction']['request'] = array(
    '#prefix' => '<div id="tabs-request">',
    '#suffix' => '</div>',
  );

  $form['transaction']['request']['header'] = array(
    '#markup' => 'Request SOAP Headers',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['request']['packet'] = array(
    '#markup' => 'Request SOAP Envelope',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['request']['data'] = array();

  $form['transaction']['response'] = array(
    '#prefix' => '<div id="tabs-response">',
    '#suffix' => '</div>',
  );
  $form['transaction']['response']['header'] = array(
    '#markup' => 'Response SOAP Headers',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['response']['packet'] = array(
    '#markup' => 'Response SOAP Envelope',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['response']['data'] = array();

    // Add some markup here.
  $form['resp_spacer'] = array(
  '#markup' => '<br/><br/>',
  );

  return $form;
}

/**
 * FAPI form used to display all services that are managed.
 */
function wsdl_docs_services_viewer($form, $form_state) {
  drupal_set_title(t("Web Services"));

    // @todo: Find and list all published services
    return $form;
}

/**
 * Get ref no by calling api.
 */
function get_reference_number($form, $form_state) {
  /*  replacing 'referenceNumberRequest' with $ref_Num_Request - Gurpreet
  return $form['parameters']['referenceNumberRequest']['referenceNumber'];
  */

  $ref_Num_Request = $_SESSION['acoperations']['operations']['request'];
  return $form['parameters'][$ref_Num_Request]['referenceNumber'];
}
/**
 * Callback for the test from 'prepare' button.
 */
function wsdl_docs_prepare_request_callback($form, $form_state) {
  $service = $form_state['build_info']['args'][0];
  $operation = $form_state['build_info']['args'][1];

  // Convert the form values into a data structure suitable for making the query.
  // Magic?
  // service::invoke will cast this array into the correct paramaterized objects
  // According to the $operation['parameter'] schema. Nifty.
  $args = array();
  if (!empty($form_state['values']['parameters'])) {
    $args = $form_state['values']['parameters'];
  }

  if ($service->type == 'soap') {
    // The service will have an endpoint that will have a SOAPClient.
    // Settings on the service->options may be passed to the SOAPClient.
    // @see WSClientSOAPEndpoint::client()
    // @see http://www.php.net/manual/en/soapclient.soapclient.php
    $service->settings['options']['trace'] = TRUE;
    // disable cache when testing!
    $service->settings['options']['cache_wsdl'] = WSDL_CACHE_NONE;
  }

  // Ready to actually invoke the call
  $timer_before = microtime();
  try {
    if ($operation['name']) {
      $response = $service->invoke($operation['name'], $args);
    }
    elseif ($operation['label']) {
      $response = $service->invoke($operation['label'], $args);
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Exception: %e'), array('%e' => $e->getMessage()));
  }
  $timer_duration = $timer_before - microtime();
  $element = $form['transaction'];

  // Place the trace data into the display.
  if ($service->type == 'rest') {
    $operation = $service->operations[$operation['name']];
    $parameters = '';
    foreach ($args as $para_name => $para_value) {
      $parameters .= $para_name . '=' . $para_value . '<br>';
    }
    $reqInfo = 'Uri:' . $service->url . $operation['url'] . '<br>' .
      'Method:' . (isset($operation['type']) ? $operation['type'] : 'GET') . '<br>' .
      'Parameter:' . $parameters;
    $element['request']['packet']['#markup'] = $reqInfo;
    $element['response']['header']['#markup'] = $service->endpoint()->client()->lastResponse->headers;
    $element['response']['packet']['#markup'] = $service->endpoint()->client()->lastResponse->body;
  }
  elseif ($service->type == 'soap') {
    $element['request']['header']['#markup'] = $service->endpoint()->client()->__getLastRequestHeaders();
    $element['request']['packet']['#markup'] = htmlspecialchars(wsdl_docs_prettify_xml($service->endpoint()->client()->__getLastRequest()));
    $_SESSION['lastRequest'] = $service->endpoint()->client()->__getLastRequest();

    $element['response']['header']['#markup'] = $service->endpoint()->client()->__getLastResponseHeaders();
    $element['response']['packet']['#markup'] = htmlspecialchars(wsdl_docs_prettify_xml($service->endpoint()->client()->__getLastResponse()));
    $_SESSION['lastResponse'] = $service->endpoint()->client()->__getLastResponse();
  }

  $element['#value'] = t("Ran at %time, took %duration to execute", array(
    '%time' => time(),
    '%duration' => $timer_duration,
  ));

  return $element;
}

/**
 * Indent and format XML for display.
 */
function wsdl_docs_prettify_xml($xml) {
  $dom = new DOMDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);
  $dom->formatOutput = TRUE;
  return $dom->saveXml();
}

/**
 * Generates a row in the properties table.
 */
function wsdl_docs_soapclient_ui_property_row($service, $types, $name = '', $info = array()) {
  $property['name'] = array(
    '#type' => 'textfield',
    '#disabled' => TRUE,
    '#size' => 40,
    '#default_value' => $name,
  );
  return $property;
}

/**
 * Generates a row in the properties table.
 */
function wsdl_docs_soapclient_ui_property_row_new($name) {
  $property['name'] = array(
    '#type' => 'textfield',
    '#disabled' => TRUE,
    '#size' => 40,
    '#default_value' => $name,
  );
  return $property;
}

/**
 * A mini form element representing the request data type.
 * textfield for most things, but nested fieldsets for complex types.
 *
 * This recurses through complex types until it hits core types.
 *
 * @return array
 *  FAPI form fragment.
 */
function wsdl_docs_request_data($label, $type, $description, $data, $datatypes, $required) {
  if (isset($datatypes[$type])) {
    // Build a complex type.
    $datatype = $datatypes[$type];
    $element = array(
      '#type' => 'fieldset',
      '#title' => check_plain("$label ({$datatype['label']})"),
      '#collapsible' => TRUE,
    );

    foreach ($datatype['property info'] as $field_id => $field_info) {
      // Recurse and get each bit to render its own input element.
      //pre-populate fields with developer data
      if ($field_id == "timeStamp") {
        $dt = new DateTime();
        $dt->setTimeZone(new DateTimeZone('UTC'));
        $timeStamp = $dt->format('c');
        $element[$field_id] = wsdl_docs_request_data($field_id, $field_info['type'], @$field_info['description'], $timeStamp, $datatypes, @$field_info['required']);
      }
      else {
        $element[$field_id] = wsdl_docs_request_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes, @$field_info['required']);
      }
    }
    return $element;
  }
  elseif (preg_match('/^list\<(.*)\>$/', $type, $matches)) {
    // Strange notation, type="list<MyType>" means a list of those things.
    // @see wsclient_soap_type_mapper()
    // This becomes a numerically indexed array.
    // Present it in the form as a nested list.
    $actual_type = $matches[1];
    $element = array(
      '#type' => 'fieldset',
      '#title' => t("List of %label (%type)", array('%label' => $label, '%type' => $type)),
      '#collapsible' => TRUE,
    );
    for ($field_id = 0; $field_id < 3; $field_id++) {
      // Recurse and get each bit to render its own input element
      $element[$field_id] = wsdl_docs_request_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes);
      $element[$field_id]['#collapsed'] = TRUE;
    }
    return $element;
  }
  elseif ($type == 'text_formatted') {
    return array(
      '#type' => 'textarea',
      '#title' => t("Property: %label", array('%label' => $label)),
      '#default_value' => $data,
      '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
    );
  }
  elseif ($type == 'boolean') {
    if (isset($required)) {
      if ($required == 'true') {
        return array(
          '#type' => 'select',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#default_value' => $data,
          // Defining false first so it's the default when setting up a form.
          '#options' => array('0' => 'false', '1' => 'true', '' => '(Not Set)'),
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => TRUE,
        );
      }
      elseif ($required == 'not allowed') {
        return array(
          '#type' => 'select',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#default_value' => NULL,
          '#value' => NULL,
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => FALSE,
          '#disabled' => TRUE,
        );
      }
      else{
      //set to optional
        return array(
          '#type' => 'select',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#default_value' => $data,
          // Defining false first so it's the default when setting up a form.
          '#options' => array('0' => 'false', '1' => 'true', '' => '(Not Set)'),
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => FALSE
        );
      }
    }
    else{
      // Set to optional.
    return array(
      '#type' => 'select',
      '#title' => t("Property: %label", array('%label' => $label)),
      '#default_value' => $data,
      // Defining false first so it's the default when setting up a form.
      '#options' => array('0' => 'false', '1' => 'true', '' => '(Not Set)'),
      '#description' => t("Type: %type <br> Description: %description <br> Required: (Not Set)", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
    );
    }

  }
  else {
    // A textfield will normally do for any other primitives.
    if (isset($required)) {
      if ($required == 'true') {
        return array(
          '#type' => 'textfield',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => TRUE,
          '#default_value' => $data,
          '#size' => 20
        );
      }
      elseif ($required == 'not allowed') {
        //set to optional
        return array(
          '#type' => 'textfield',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => FALSE,
          '#disabled' => TRUE,
          '#default_value' => NULL,
          '#value' => NULL,
          '#size' => 20
        );
      }
      else {
      //required is set to a value other than true or not allowed so field is optional
        return array(
          '#type' => 'textfield',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => FALSE,
          '#default_value' => $data,
          //'#value' => NULL, //is_null('#value') ? '#value' : NULL,
          '#size' => 20
        );
      }
    }
    else{
      //required parameter not set so field is optional
      return array(
        '#type' => 'textfield',
        '#title' => t("Property: %label", array('%label' => $label)),
        '#description' => t("Type: %type <br> Description: %description <br> Required: (Not Set)", array('%label' => $label, '%type' => $type, '%description' => $description)),
        '#required' => FALSE,
        '#default_value' => $data,
        //'#value' => NULL,
        '#size' => 20
      );
    }
  }
}

/**
 * A mini form element representing the response data type.
 * textfield for most things, but nested fieldsets for complex types.
 *
 * This recurses through complex types until it hits core types.
 *
 * @return array
 *  FAPI form fragment.
 */
function wsdl_docs_response_data($label, $type, $description, $data, $datatypes) {
  if (isset($datatypes[$type])) {
    // Build a complex type.
    $datatype = $datatypes[$type];
    $element = array(
      '#type' => 'fieldset',
      '#title' => check_plain("{$datatype['label']}"),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    foreach ($datatype['property info'] as $field_id => $field_info) {
      // Recurse and get each bit to render its own input element.
      $element[$field_id] = wsdl_docs_response_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes);
    }
    return $element;
  }
  elseif (preg_match('/^list\<(.*)\>$/', $type, $matches)) {
    // Strange notation, type="list<MyType>" means a list of those things.
    // @see wsclient_soap_type_mapper()
    // This becomes a numerically indexed array.
    // Present it in the form as a nested list.
    $actual_type = $matches[1];
    $element = array(
      '#type' => 'fieldset',
      '#title' => t("List of %label (%type)", array('%label' => $label, '%type' => $type)),
      '#collapsible' => TRUE,
    );
    for ($field_id = 0; $field_id < 3; $field_id++) {
      // Recurse and get each bit to render its own input element
      $element[$field_id] = wsdl_docs_response_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes);
      $element[$field_id]['#collapsed'] = TRUE;
    }
    return $element;
  }
  elseif ($type == 'text_formatted') {
    return array(
      '#markup' => t("<b>Property: %label</b> <br>Type: %type <br>Description: %description<br>", array('%label' => $label, '%type' => $type, '%description' => $description)),
    );
  }
  elseif ($type == 'boolean') {
    //set to optional
    return array(
      '#markup' => t("<b>Property: %label</b> <br>Type: %type <br>Description: %description<br>", array('%label' => $label, '%type' => $type, '%description' => $description)),
    );
  }
  else {
    // A textfield will normally do for any other primitives.
    return array(
      '#markup' => t("<b>Property: %label</b> <br>Type: %type <br>Description: %description<br>", array('%label' => $label, '%type' => $type, '%description' => $description)),
    );
  }
}

/**
 * @param $title
 * @param $node_type
 * @return bool|mixed
 */
function wsdl_docs_node_load_by_title($title, $node_type) {
  $query = new EntityFieldQuery();
  $entities = $query->entityCondition('entity_type', 'node')
      ->propertyCondition('type', $node_type)
      ->propertyCondition('title', $title)
      ->range(0, 1)
      ->execute();
  if (!empty($entities)) {
    return node_load(array_shift(array_keys($entities['node'])));
  }
}

/**
 * Create a new WSDL Docs service via REST.
 * Will need to add the reference to the WSDL/XSD documentation in a separate PUT/POST.
 *
 * @param array $data
 *  Array of body data passed via POST
 * @return string
 */
function _wsdl_docs_service_create($data) {
  // Confirm service name is unique
  $service_id = _wsdl_docs_get_service_id($data['name']);
  if ($service_id) {
    return "Service with this name already exists";
  }
  // @todo - this duplicates wsdl_docs_create_wsclient_service() but we need to do something a little different here.
  // Create new WSDL service.
  $service = entity_create('wsclient_service', array('type' => 'soap'));
  $service->label = $data['name'];
  $service->name = strtolower(str_replace('-', '_', str_replace(' ', '_', $data['name'])));
  $service->save();
  // Create WSDL service's operation nodes.
  // wsdl_docs_update_wsclient_service($service);
  $name = $data['name'];
  $id = $service->id;
  return "WSDL service $name, ID: $id added.";
}

/**
 * Update an existing WSDL Docs service via REST PUT.
 *
 * @param string $name
 *  Name of the WSDL Docs service being updated
 * @param array $data
 *  Body data contains WSDL service URL
 * @return string
 *  Success or failure of update request.
 */
function _wsdl_docs_service_update($name, $data) {
  $service_id = _wsdl_docs_get_service_id($name);
  if (is_numeric($service_id)) {
    // load existing WSDL service
    $service = entity_load_single('wsclient_service', $service_id);
    // Update existing WSDL service's URL.
    $service->url = $data['url'];
    // Validate wsdl before saving.
    _wsdl_docs_validate_wsdl(drupal_realpath($service->url));
    $service->save();
    // update WSDL service's operation nodes
    $wsdl = file_get_contents($data['url']);
    $DOMDocument = new DOMDocument();
    $DOMDocument->loadXML($wsdl);
    wsdl_docs_update_wsclient_service($service, $DOMDocument);
    return "UPDATE SERVICE: $name, ID: $service_id";
  }
  return "WSDL Doc not updated. No WSDL Doc matching the name and resource url was found.";
}

/**
 * Delete WSDL Docs service via REST DELETE.
 *
 * @param $name
 *  Name of the WSDL Docs service being deleted
 * @param $data
 *  Array of body data passed via DELETE
 * @return string
 *  Success or failure of DELETE request.
 */
function _wsdl_docs_service_delete($name) {
  $service_id = _wsdl_docs_get_service_id($name);
  if (is_numeric($service_id)) {
    entity_delete('wsclient_service', $service_id);
    return "DELETE SERVICE: $name, ID: $service_id";
  }
  return "WSDL Doc not deleted. No WSDL Doc matching the name was found.";
}

/**
 * Given a WSDL Doc service, get its ID.
 *
 * @param string $name
 *  Name of the WSDL Docs service being updated
 * @param string $url
 *  URL of WSDL Doc.
 * @return string|NULL mixed
 *  Service ID or NULL if not found.
 */
function _wsdl_docs_get_service_id($name) {
  // Get service ID by service name and url.
  // @todo what happens if there are 2 services with the same name?
  $service_id = db_select('wsclient_service', 'ws')
    ->fields('ws', array('id'))
    ->condition('ws.name', $name)
    ->execute()
    ->fetchField();
  return $service_id;
}

/**
 * Import file(s) to an existing WSDL Doc service via REST POST.
 *
 * @param $name
 *  Name of the WSDL Docs service being updated/created
 * @return mixed|string
 */
function _wsdl_docs_service_import($name) {
  // Get existing service ID.
  $service_id = _wsdl_docs_get_service_id($name);
  if (is_numeric($service_id)) {
    // Load existing service.
    $service = entity_load_single('wsclient_service', $service_id);
  }
  else {
    return "No service found with this name.";
  }
  // Create $_FILES['files'] from array of file information provided by REST request.
  $_FILES['files'] = array();
  foreach ($_FILES['soap_api_definition'] as $file_key => $values) {
    foreach ($values as $key => $value) {
      $_FILES['files'][$key][$file_key] = $value;
    }
  }
  // Confirm imported file(s) are of correct extension(s).
  foreach ($_FILES['files'] as $key => $file) {
    $ext = pathinfo($file['name']);
    if (!in_array($ext['extension'], array('xsd', 'wsdl'))) {
      return t('incorrect file format');
    }
  }
  // Save file(s) to selected directory.
  $directory = WSDL_DIRECTORY;
  file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
  $multiple_files = (count($_FILES['files']) > 1);
  // Prepare to zip up multiple files for download from admin/content/wsdl_docs.
  if ($multiple_files) {
    $zip = new ZipArchive();
    $zip_filename = $service->name . '-' . $service->id . '.zip';
    $path = drupal_realpath($directory) . '/' . $zip_filename;
    fopen(drupal_realpath($zip_filename), 'w');
    $zip->open($path, ZipArchive::CREATE);
  }
  // Save individual file(s) as well as add to zip if multiple files imported.
  $i = 0;
  foreach ($_FILES['files'] as $key => $file) {
    $wsdl = file_get_contents($file['tmp_name']);
    // For the main WSDL/XSD file that refers to other files, name it
    // [service name]-[service id].wsdl or .xsd, depending on file's extension.
    // We are assuming this file has been provided first in the list of files to import.
    if ($i == 0) {
      $ext = pathinfo($file['name']);
      $tmp_filename = WSDL_DIRECTORY . $service->name . '-' . $service->id . '.' . $ext['extension'];
      $main_filename = $tmp_filename;
    }
    // For any additional files, keep the name the same so references from
    // the main file still work.
    else {
      $tmp_filename = WSDL_DIRECTORY . $file['name'];
    }
    // Validate wsdl before saving.
    _wsdl_docs_validate_wsdl(drupal_realpath($tmp_filename));
    file_unmanaged_save_data($wsdl, $tmp_filename, FILE_EXISTS_REPLACE);
    // For multiple files, add this file to the zip.
    if ($multiple_files) {
      $zip->addFromString($file['name'], $wsdl);
    }
    // Increment counter as we only rename the first file passed in the request.
    $i++;
  }
  // Save path of file to existing WSDL service entity.
  if (isset($main_filename)) {
    $service->url = $main_filename;
    $service->save();
    $wsdl = file_get_contents($main_filename);
    $DOMDocument = new DOMDocument();
    $loaded = $DOMDocument->loadXML($wsdl);
    if (!$loaded) {
      return 'ERROR input file is not XML';
    }
    // Update existing service with new specs per file upload.
    wsdl_docs_update_wsclient_service($service, $DOMDocument);
  }
  else {
    return 'Error importing files.';
  }
  return "WSDL Doc: $name, id: $service_id updated.";
}

/**
 * Validates that the URL (either local path to file or external URL)
 * points to a valid WSDL file.
 *
 * This was taken from wsclient_soap.module so validation may be run twice,
 * but wanted to validate WSDL before writing the file to the private directory.
 *
 * @param $url
 *  path to local wsdl file imported by PUT or remote URL
 *
 * @return string|null
 *  Error if exception is caught, null if no exception.
 */
function _wsdl_docs_validate_wsdl($url) {
  // The url has to point to a valid WSDL file.
  try {
    // If initializing the SOAPClient succeeds we're good, otherwise we catch
    // the exception below and suppress any further warnings.
    // WARNING: if you have the xdebug PHP module enabled this can cause a
    // fatal error on invalid WSDL files (instead of a catchable SoapFault
    // exception).
    // xdebug_disable();
    @$endpoint = new SOAPClient(drupal_realpath($url));
  }
  catch (SoapFault $e) {
    $error = $e->getMessage();
    return "Error parsing the WSDL file: $error";
  }
  return NULL;
}
