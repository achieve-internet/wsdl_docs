<?php

//libraries_load("jsonpath");
/**
 * @file
 * Utility functions for displaying and testing SOAP services
 */

/**
 * FAPI form used to display the options and the results of calling a web
 * service.
 */
function smartdocs_wsdl_operation_invoke($form, $form_state, $service, $operation) {
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

  //custom css for form parameters
  $form['parameters']['#attached']['css'][] = drupal_get_path('module', 'smartdocs_wsdl') . '/ui/ui.css'; 
  
  // Deal with complex types.
  // Each complex type may require its own mini-form for data entry,
  // and these may be nested.
  $datatypes = $service->datatypes;
  foreach ($operation['parameter'] as $param => $info) {
      $form['parameters'][$param] = smartdocs_wsdl_request_data($param, $info['type'], @$info['description'], @$form_state['values']['parameters'], $datatypes, @$info['required']);
  }

  $form['execute'] = array(
    '#type' => 'submit',
    '#value' => 'Execute Request',
    '#ajax' => array(
      'event' => 'click',
      'callback' => 'smartdocs_wsdl_prepare_request_callback',
      'wrapper' => 'edit-transaction',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );

  //add some markup here
  $form['spacer'] = array(
  '#markup' => '<br/><br/>',
  );

  // Set up result panes. Content for these usually gets filled in via ajax.

  $form['transaction'] = array(
    '#type' => 'fieldset',
    '#title' => 'Transaction',
    '#attributes' => array('id' => 'edit-transaction'),
  );

  $form['transaction']['request'] = array(
    '#type' => 'fieldset',
    '#title' => 'Request',
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
    '#type' => 'fieldset',
    '#title' => 'Response',
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

    //add some markup here
  $form['resp_spacer'] = array(
  '#markup' => '<br/><br/>',
  );

  return $form;
}

/**
 * FAPI form used to display the options and the results of calling a web
 * service during the certification process.
 */
function smartdocs_wsdl_certify_operation_invoke($form, &$form_state, $service, $certification, $certification_step) {
  $certs = array();
  $operations = $service->operations;
  $operation = NULL;

  $strings = array(
    '!service_label' => $service->label,
    '!operation_label' => $operation['label'],
  );
  drupal_set_title(t("!service_label : !operation_label()", $strings));
  $form  = array();
  
  $form['header-title'] = array(
  '#markup' => '<h2>' . $service->label . " : " . $operation['label'] . '</h2><br>',
  );
  
/*  $form['header-description'] = array(
  '#markup' => $operation['certification'][$certification]['description'],
  ); */   //Commented by Gurpreet
  
  $form['header-description'] = array(
  '#markup' => $descriptionField,
  );
  
  $form['parameters'] = array(
    '#title' => 'Parameters',
    '#type' => 'container',
    '#tree' => TRUE,
    '#value' => t("Enter the parameters to the !operation_label service here. It's up to you to get the data types right. No type validation is done at this end, as it's a debugger to let you throw various errors at the web service and see how it responds.", $strings),
  );

  //custom css for form parameters
  $form['parameters']['#attached']['css'][] = drupal_get_path('module', 'smartdocs_wsdl') . '/ui/ui.css';   
  // Deal with complex types.
  // Each complex type may require its own mini-form for data entry,
  // and these may be nested.
  $datatypes = $service->datatypes;
  foreach ($operation['parameter'] as $param => $info) {
      $form['parameters'][$param] = smartdocs_wsdl_certify_data($param, $info['type'], @$info['description'], @$form_state['values']['parameters'], $datatypes, @$info['required'], $excludeFields, $defaultValues);
  }

  $form['parameters'][$ref_Num_Request]['referenceNumber']['#prefix'] = '<div id="replace_reference_number_value">';
  $form['parameters'][$ref_Num_Request]['referenceNumber']['#suffix'] =  '</div>';

  
  $form['execute'] = array(
    '#type' => 'submit',
    '#value' => 'Execute Request',
    '#ajax' => array(
      'event' => 'click',
      'callback' => 'smartdocs_wsdl_prepare_request_callback',
      'wrapper' => 'edit-transaction',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );
  
  //add some markup here
  $form['spacer'] = array(
  '#markup' => '<br/><br/>',
  );

  // Set up result panes. Content for these usually gets filled in via ajax.

  $form['transaction'] = array(
    '#type' => 'fieldset',
    '#title' => 'Transaction',
    '#attributes' => array('id' => 'edit-transaction'),
  );

  $form['transaction']['request'] = array(
    '#type' => 'fieldset',
    '#title' => 'Request',
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
    '#type' => 'fieldset',
    '#title' => 'Response',
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

    //add some markup here
  $form['resp_spacer'] = array(
  '#markup' => '<br/><br/>',
  );

  return $form;
}

/**
 * FAPI form used to display the properties of a datatype
 */
function smartdocs_wsdl_type_viewer($form, $form_state, $service, $type){
  $strings = array(
  '!service_label' => $service->label,
  '!type_label' => $type['label'],
  );

  drupal_set_title(t("!service_label : !type_label()", $strings));
  $form  = array();

  $form['header-title'] = array(
  '#markup' => '<h2>' . $type['name'] . '</h2><br>',
  );

   $rows = array();
    $types = soapclient_datatypes();
    foreach ($type['property info'] as $name => $info) {
      $row = array();
      $row[] = array('data' => array(
        '#theme' => 'entity_ui_overview_item',
        '#label' => $name,
        '#name' => $name,
      ));

      $rows[] = $row;
    }

    $row = array();
    // @todo description help text for operations, data types
    $form['types'] = array(
      '#access' => TRUE,
      '#tree' => TRUE,
      '#theme' => 'table',
      '#empty' => t('None'),
      '#caption' => t('Properties'),
      '#rows' => $rows,
      '#header' => t('Properties'),
      '#weight' => 50,
    );
    // Add some table styling from Rules.
    $form['types']['#attributes']['class'][] = 'table table-striped';
    $form['types']['#attached']['css'][] = drupal_get_path('module', 'smartdocs_wsdl') . '/ui/ui.css';

  return $form;
}

/**
 * FAPI form used to display all operations and datatypes of a service
 */
function smartdocs_wsdl_operation_viewer($form, $form_state, $service) {

  $strings = array(
    '!service_label' => $service->label,
  );
  drupal_set_title(t("Testing Web Service : !service_label", $strings));

  $type_info = wsclient_get_types();
    if (empty($type_info)) {
      drupal_set_message(t('No service types were found, please enable a module that provides a service type.'), 'warning');
    }
    $types = array();
    foreach ($type_info as $type => $info) {
      $types[$type] = $info['label'];
    }

      $form['header-title'] = array(
        '#markup' => '<h2>' . $service->label . '</h2><br>',
        );

      $rows = array();
      $operations = smartdocs_wsdl_label_sort($service->operations);
      foreach ($operations as $name => $operation) {
        $row = array();
        $row[] = array('data' => array(
          '#theme' => 'entity_ui_overview_item',
          '#label' => $operation['label'],
          '#name' => $name,
          '#url' => array(
            'path' => '/soap/' . $service->name . '/operation/' . $name . '/invoke',
            'options' => array(),
          ),
        ));

        $rows[] = $row;
      }

      $row = array();
      // @todo description help text for operations, data types
      $form['operations'] = array(
        '#access' => TRUE,
        '#tree' => TRUE,
        '#theme' => 'table',
        '#empty' => t('None'),
        '#caption' => t('Operations'),
        '#rows' => $rows,
        '#header' => $header,
        '#weight' => 50,
      );
      // Add some table styling from Rules.
      $form['operations']['#attributes']['class'][] = 'table table-striped';
      $form['operations']['#attached']['css'][] = drupal_get_path('module', 'smartdocs_wsdl') . '/ui/ui.css';



      // Data types of the web service in a table
      $rows = array();
      $datatypes = smartdocs_wsdl_label_sort($service->datatypes);
      foreach ($datatypes as $name => $datatype) {
        $row = array();
        $row[] = array('data' => array(
          '#theme' => 'entity_ui_overview_item',
          '#label' => $datatype['label'],
          '#name' => $name,
          '#url' => array(
            'path' => '/soap/' . $service->name . '/type/' . $name,
            'options' => array(),
          ),
        ));
        $rows[] = $row;
      }

      $row = array();

      $form['datatypes'] = array(
        '#access' => TRUE,
        '#tree' => TRUE,
        '#theme' => 'table',
        '#empty' => t('None'),
        '#caption' => t('Data types'),
        '#rows' => $rows,
        '#header' => $header,
        '#weight' => 60,
      );
      $form['datatypes']['#attributes']['class'][] = 'table table-striped';
      $form['datatypes']['#attached']['css'][] = drupal_get_path('module', 'smartdocs_wsdl') . '/ui/ui.css';

    return $form;
}

/**
 * FAPI form used to display all services that are managed
 */
function smartdocs_wsdl_services_viewer($form, $form_state) {
  drupal_set_title(t("Web Services"));

    //to-do: Find and list all published services
    return $form;
}

/**
* get ref no by calling api
*/
function get_reference_number($form, $form_state){
  /*  replacing 'referenceNumberRequest' with $ref_Num_Request - Gurpreet 
  return $form['parameters']['referenceNumberRequest']['referenceNumber'];
  */
  
  $ref_Num_Request = $_SESSION['acoperations']['operations']['request'];
  return $form['parameters'][$ref_Num_Request]['referenceNumber'];
}
/**
 * Callback for the test from 'prepare' button.
 */
function smartdocs_wsdl_prepare_request_callback($form, $form_state) {
 // firep($form_state);
  $service = $form_state['build_info']['args'][0];
  if ($form_state['build_info']['form_id'] === 'smartdocs_wsdl_certify_operation_invoke'){
      //look up operation
    $operations = $service->operations;
    $certification = $form_state['build_info']['args'][1];
    $certification_step = $form_state['build_info']['args'][2];
  
  /*    Changed the ForEach condition to use operation name instead of the certification_step counter (Gurpreet) - START
    foreach ($operations as $op => $opinfo) {
      if ($opinfo['certification'][$certification]['index'] === $certification_step){
        $operation = $operations[$op];
      }
    }     Changed the ForEach condition to use operation name instead of the certification_step counter (Gurpreet) - END */
  
  //BLOCK1: Use the session variable to get the operation name for comparison inside foreach (Gurpreet) - START
  //session variable set in smartdocs_wsdl_certify_operation_invoke function
  foreach ($operations as $op => $opinfo) {
      if ($opinfo['label'] === $_SESSION['acoperations']['operations']['name']){
        $operation = $operations[$op];
      }
    }
  //BLOCK1: Use the session variable to get the operation name inside foreach condition (Gurpreet) - START
  }
  else {
    $operation = $form_state['build_info']['args'][1];

  }

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
    if($operation['name']){
    $response = $service->invoke($operation['name'], $args);
  }
    elseif($operation['label']){
      $response = $service->invoke($operation['label'], $args);
    }
  }
  catch (Exception $e) {
    //$response = $e->getMessage();
  drupal_set_message("<pre>Catch Exception - " . $e->getMessage() . "</pre>");
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
    $element['request']['packet']['#markup'] = htmlspecialchars(smartdocs_wsdl_prettify_xml($service->endpoint()->client()->__getLastRequest()));
    $_SESSION['lastRequest'] = $service->endpoint()->client()->__getLastRequest();

    $element['response']['header']['#markup'] = $service->endpoint()->client()->__getLastResponseHeaders();
    $element['response']['packet']['#markup'] = htmlspecialchars(smartdocs_wsdl_prettify_xml($service->endpoint()->client()->__getLastResponse()));
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
function smartdocs_wsdl_prettify_xml($xml) {
  $dom = new DOMDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);
  $dom->formatOutput = TRUE;
  return $dom->saveXml();
}

/**
 * Helper function to sort a nested data information array based on the label
 * of the items.
 */
function smartdocs_wsdl_label_sort($data_info, $label_key = 'label') {
  $sort_info = array();
  foreach ($data_info as $key => $info) {
    $sort_info[$key] = $info[$label_key];
  }
  natcasesort($sort_info);
  foreach ($sort_info as $key => $label) {
    $sort_info[$key] = $data_info[$key];
  }
  return $sort_info;
}

/**
 * Returns a list of available data types for service parameters, return
 * values or data type properties. Entities are excluded.
 *
 * @param $hidden
 *   Flag to indicate whether the special 'hidden' data type should be added to
 *   the list as well.
 */
function soapclient_datatypes($hidden = FALSE) {
  $cache = rules_get_cache();
  $data_info = $cache['data_info'];
  $entity_info = entity_get_info();
  // Remove entities.
  $data_info = array_diff_key($data_info, $entity_info);
  unset($data_info['entity']);
  // Remove the generic list type.
  unset($data_info['list']);
  $options = array();
  foreach ($data_info as $type => $properties) {
    // Do not add lists as this handled by the "multiple" checkbox.
    if (strpos($type, 'list<') !== 0) {
      $options[$type] = $properties['label'];
    }
  }
  if ($hidden) {
    // Add special 'hidden' data type
    $options['hidden'] = t('hidden');
  }
  natcasesort($options);
  return $options;
}

/**
 * Generates a row in the properties table.
 */
function _soapclient_ui_property_row($service, $types, $name = '', $info = array()) {
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
function _soapclient_ui_property_row_new($name) {
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
 * @return a FAPI form fragment.
 */
function smartdocs_wsdl_request_data($label, $type, $description, $data, $datatypes, $required) {
  if (isset($datatypes[$type])) {
    // Build a complex type.
    $datatype = $datatypes[$type];
    $element = array(
      '#type' => 'fieldset',
      '#title' => check_plain("$label ({$datatype['label']})"),
      '#collapsible' => TRUE,
    );

      global $user;
      $currentUser = $user->mail;
      $developer = entity_load('developer', array($currentUser), array(), TRUE);
      $attributes = $developer[$currentUser]->attributes;
      //dpm($attributes, "attributes");

    foreach ($datatype['property info'] as $field_id => $field_info) {
      // Recurse and get each bit to render its own input element.
      //pre-populate fields with developer data
      if (array_key_exists($field_id, $attributes))
        {
          $one = $attributes[$field_id];
     //     firep("$field_id - true - $one");
          $element[$field_id] = smartdocs_wsdl_request_data($field_id, $field_info['type'], @$field_info['description'], $attributes[$field_id], $datatypes, @$field_info['required']);
        }
      elseif($field_id == "timeStamp")
        {
          $dt = new DateTime();
          $dt->setTimeZone(new DateTimeZone('UTC'));
          $timeStamp = $dt->format('c');
          $element[$field_id] = smartdocs_wsdl_request_data($field_id, $field_info['type'], @$field_info['description'], $timeStamp, $datatypes, @$field_info['required']);
        }
      else
        {
          $two = $attributes[$field_id];
        //  firep("$field_id - false - $two ");
          $element[$field_id] = smartdocs_wsdl_request_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes, @$field_info['required']);
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
      $element[$field_id] = smartdocs_wsdl_request_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes);
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

    if (isset($required)){
      if ($required == 'true')
      {
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
      else if ($required == 'not allowed') {
        return array(
          '#type' => 'select',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#default_value' => NULL,
          '#value' => NULL,
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => FALSE,
          '#disabled' => TRUE,
          '#value' => NULL,
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
      //set to optional
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
    if (isset($required)){
      if ($required == 'true')
      {
         return array(
          '#type' => 'textfield',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => TRUE,
          '#default_value' => $data,
          '#size' => 20
        );
      }
      else if ($required == 'not allowed') {
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
      else{
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

function smartdocs_wsdl_certify_data($label, $type, $description, $data, $datatypes, $required, $excludeFields, $defaultValues) {

  if (isset($datatypes[$type])) {
    // Build a complex type.
    $datatype = $datatypes[$type];
    $element = array(
      '#type' => 'fieldset',
      '#title' => check_plain("$label ({$datatype['label']})"),
      '#collapsible' => TRUE,
    );

    global $user;
    $currentUser = $user->mail;
    $developer = entity_load('developer', array($currentUser), array(), TRUE);
    $devattr = $developer[$currentUser]->attributes;

    if ($defaultValues) { 
      $attributes = array_merge($devattr,$defaultValues);
   }
    else{
      $attributes = $devattr;
    }

    foreach ($datatype['property info'] as $field_id => $field_info) {
     if (in_array($field_id, $excludeFields))
      {
        //skip
        //return null;
      }
      // Recurse and get each bit to render its own input element.
      //pre-populate fields with developer data
      elseif (array_key_exists($field_id, $attributes))
      {
        $element[$field_id] = smartdocs_wsdl_certify_data($field_id, $field_info['type'], @$field_info['description'], $attributes[$field_id], $datatypes, @$field_info['required'], $excludeFields, $defaultValues);
      }
      elseif($field_id == "timeStamp")
      {
        $dt = new DateTime();
        $dt->setTimeZone(new DateTimeZone('UTC'));
        $timeStamp = $dt->format('c');
        $element[$field_id] = smartdocs_wsdl_certify_data($field_id, $field_info['type'], @$field_info['description'], $timeStamp, $datatypes, @$field_info['required'], $excludeFields, $defaultValues);
      }
      else
      {
       $element[$field_id] = smartdocs_wsdl_certify_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes, @$field_info['required'], $excludeFields, $defaultValues);
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
      $element[$field_id] = smartdocs_wsdl_certify_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes, @$field_id['required'], $excludeFields, $defaultValues);
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

    if (isset($required)){
      if ($required == 'true')
      {
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
      else if ($required == 'not allowed') {
        return array(
          '#type' => 'select',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#default_value' => NULL,
          '#value' => NULL,
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => FALSE,
          '#disabled' => TRUE,
          '#value' => NULL,
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
      //set to optional
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
    if (isset($required)){
      if ($required == 'true')
      {
        return array(
          '#type' => 'textfield',
          '#title' => t("Property: %label", array('%label' => $label)),
          '#description' => t("Type: %type <br> Description: %description <br> Required: %required", array('%label' => $label, '%type' => $type, '%description' => $description, '%required' => $required)),
          '#required' => TRUE,
          '#default_value' => $data,
          '#size' => 20
        );
      }
      else if ($required == 'not allowed') {
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
      else{
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
 * @return a FAPI form fragment.
 */
function smartdocs_wsdl_response_data($label, $type, $description, $data, $datatypes) {
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
      $element[$field_id] = smartdocs_wsdl_response_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes);
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
      $element[$field_id] = smartdocs_wsdl_response_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes);
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

function smartdocs_wsdl_node_load_by_title($title, $node_type) {
    $query = new EntityFieldQuery();
    $entities = $query->entityCondition('entity_type', 'node')
        ->propertyCondition('type', $node_type)
        ->propertyCondition('title', $title)
        ->range(0,1)
        ->execute();
    if(!empty($entities)) {
        return node_load(array_shift(array_keys($entities['node'])));
    }
}