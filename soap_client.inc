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
function soap_client_operation_invoke($form, $form_state, $service, $operation) {
  $strings = array(
    '!service_label' => $service->label,
    '!operation_label' => $operation['label'],
  );
  drupal_set_title(t("!service_label : !operation_label()", $strings));
  $form  = array();
  
  $form['header-title'] = array(
  '#markup' => '<h2>Operation: ' . $operation['label'] . '</h2><br>',
  );

  /* hide this for now
    $form['uri'] = array(
      '#title' => 'Service URI',
      '#type' => 'textfield',
      '#disabled' => TRUE,
      '#value' => $service->url,
    );
  */

  $form['parameters'] = array(
    '#title' => 'Parameters',
    '#type' => 'container',
    '#tree' => TRUE,
    '#value' => t("Enter the parameters to the !operation_label service here. It's up to you to get the data types right. No type validation is done at this end, as it's a debugger to let you throw various errors at the web service and see how it responds.", $strings),
  );

  //custom css for form parameters
  $form['parameters']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css'; 
  
  // Deal with complex types.
  // Each complex type may require its own mini-form for data entry,
  // and these may be nested.
  $datatypes = $service->datatypes;
  foreach ($operation['parameter'] as $param => $info) {
      $form['parameters'][$param] = soap_client_request_data($param, $info['type'], @$info['description'], @$form_state['values']['parameters'], $datatypes, @$info['required']);
  }

  $form['execute'] = array(
    '#type' => 'submit',
    '#value' => 'Execute Request',
    '#ajax' => array(
      'event' => 'click',
      'callback' => 'wsclient_tester_prepare_request_callback',
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

  /* hide this for now
    $form['response'] = array(
      '#type' => 'fieldset',
      '#title' => 'Response Definition',
    );

    //display the result parameters
    $form['response']['results'] = array(
      '#title' => 'result',
      '#type' => 'container',
      '#tree' => TRUE,
    );
    $form['response']['results']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css'; 
   

    // Deal with complex types.
    // Each complex type may require its own mini-form for data entry,
    // and these may be nested.
    

    $datatypes = $service->datatypes;
      $form['response']['results']['definition'] = soap_client_response_data($operation['result'], $operation['result']['type'], @$resinfo['description'], @$form_state['values']['parameters'], $datatypes, @$resinfo['required']);
  */

  return $form;
}

/**
 * FAPI form used to display the options and the results of calling a web
 * service during the certification process.
 */
function soap_client_certify_operation_invoke($form, &$form_state, $service, $certification, $certification_step) {
  require_once jsonpath_parser_library_path();

  $certs = array();
  $operations = $service->operations;
  $operation = NULL;

  //Replaced with another foreach loop after the commented code block (Gurpreet) - START
  /*foreach ($operations as $op=> $opinfo) {
  
    if ($opinfo['certification'])
    {
      if ($opinfo['certification'][$certification]) {
        if ($opinfo['certification'][$certification]['index'] === $certification_step){
          $operation = $operations[$op];
      $excludeFields = explode(",", $opinfo['certification'][$certification]['excludeFields']);
          $includeFields = $opinfo['certification'][$certification]['includeFields'];
      array_push($certs, $opinfo);
          if($opinfo['certification'][$certification]['sessionID']){
            //set flag to store session
            $session_var = $opinfo['certification'][$certification]['sessionID'];
          }
        }
      }
    }
  } */
  //Replaced with another foreach loop after the commented code block (Gurpreet) - END
  
  //BLOCK1: Retreive Operations for a API Category (Gurpreet) - START
  //arg logic fixed for ref number field , Use $_SESSION['acoperations']['operations']['key'] instead of arg(3) (Shruti) - START
  if(!isset($_SESSION['acoperations']['operations']['key']) || (null !== arg(3))){
    $_SESSION['acoperations']['operations']['key'] = arg(3);
  }
  //arg logic fixed for ref number field , Use $_SESSION['acoperations']['operations']['key'] instead of arg(3) (Shruti) - END

  //$soap_operations_key = arg(3);
  $soap_operations_key = $_SESSION['acoperations']['operations']['key'];  
  $soap_operations_node =  node_load_by_title($soap_operations_key, 'acoperations');
  if ($soap_operations_node) {
    // Load the sample payload
    $soap_operations = $soap_operations_node->body[LANGUAGE_NONE][0]['value'];
    if ($soap_operations) {
      $operationsInc = explode(",", $soap_operations);
      $operationsCount = count($operationsInc);     //this variable has the number of elements in array. need to use this for loop 
      if ($operationsInc) {
		//$soap_operation_name = $operationsInc[$certification_step];		//using "with_index" variable now - Gurpreet
		$soap_operation_name_with_index = $operationsInc[$certification_step];		//using "with_index" variable to get as-is soap operation name
		
		/* Multiple operation calls with same name (Gurpreet) - START
		added logic to handle multiple operation calls for same API within single sample transaction. 
		e.g. calling receiveValidation twice within Form Free Receive Sample transaction.
		To distinguish each of these receiveValidation operations (because data for each is different), '_' is added to the operation name.
		e.g. "receiveValidation_1" & "receiveValidation_2"
		below code is to extract the actual operation/API name and drop the '_' and sequence ---  Gurpreet */
		
		$soap_operation_name_pos = strpos($soap_operation_name_with_index, "_");
		if ($soap_operation_name_pos === false) {		// string '_' not found
			$soap_operation_name = $soap_operation_name_with_index;
		}
		else {		//	string '_' found
			$soap_operation_name = substr($soap_operation_name_with_index, 0, strpos($soap_operation_name_with_index, '_'));
		}
		//Multiple operation calls with same name (Gurpreet) - END
		
		//if ($soap_operation_name) {		//using "with_index" variable now - Gurpreet
		if ($soap_operation_name_with_index) {
			$_SESSION['acoperations']['operations']['name'] = $soap_operation_name;

			//BLOCK2: Retreive include exclude fields for the current operation - Start (Gurpreet)
			$soap_operations_fields_key = $soap_operations_key . "_" . $soap_operation_name_with_index;
			$soap_operations_fields_node =  node_load_by_title($soap_operations_fields_key, 'acoperfields');
	  
			if ($soap_operations_fields_node)
			{
				$descriptionField = $soap_operations_fields_node->body[LANGUAGE_NONE][0]['value'];
				$includeFieldsObject = $soap_operations_fields_node->field_includefields[LANGUAGE_NONE][0]['value'];
				$includeFields = json_decode($includeFieldsObject,true);
				
				// for retrieving customer information from BaaS - START ***********************************
				if((arg(1) == 'agentconnect_send' && $soap_operation_name == 'sendValidation' && (arg(3) != 'adustomex' && arg(3) != 'hdsuktodom')) || (arg(1) == 'agentconnect_receive' && (int)$certification_step == 0) || (arg(1) == 'form_free' && (arg(3) == 'recusatomexformfreereceive' || arg(3) == 'usatomexformfreesend' ) && (int)$certification_step == 0) || (arg(1) == 'amend' && (int)$certification_step == 0) || (arg(1) == 'send_reversal' && (int)$certification_step == 0))
				{	
					//if(!isset($_SESSION['baasresponse']['sender']) || !isset($_SESSION['baasresponse']['receiver'])){
						$baas_payload_key = "userdetailsbaas";
						$baas_payload_node =  node_load_by_title($baas_payload_key, 'acconfig');

						if ($baas_payload_node) {
						  // Load the sample payload
							$baas_payload = $baas_payload_node->body[LANGUAGE_NONE][0]['value'];
							$baas_payload_array = json_decode($baas_payload,true);
							$baas_id_string = $baas_payload_array['ID'];
							$min_baas_index = $baas_payload_array['min_baas_index'];
							$max_baas_index = $baas_payload_array['max_baas_index'];
							$baas_url = $baas_payload_array['baas_url'];

							//*********************** Call BaaS proxy for retrieving SENDER data ******************************* START
							$baas_index_sender = rand($min_baas_index,$max_baas_index);	//generate random number between min_baas_index to min_baas_index
							$updated_baas_payload_sender = str_replace($baas_id_string, $baas_index_sender, $baas_payload);

							$baas_options_sender = array(
							  'method' => 'POST',
							  'data' => $updated_baas_payload_sender,
							  'headers' => array('Content-Type' => 'application/json'),
							  'timeout' => 30,
							);
							
							$baas_response_sender = drupal_http_request($baas_url, $baas_options_sender);
							//$baas_url	= 'https://moneygram-dev.apigee.net/demobaas'
							
							$parsed_baas_response_sender = str_replace("tempuser_","sender",$baas_response_sender->data);
							
							/* Replace senderphone with senderhomephone. This field is named different for sender and receiver. 
							Sender field name is senderHomePhone. Receiver field name is receiverPhone. No change required for receiver.
							Requirement by Paul - START */
							$parsed_baas_response_sender = str_replace("senderPhone","senderHomePhone",$parsed_baas_response_sender);
							//Replace senderphone with senderhomephone - END
							
							//*********************** Call BaaS proxy for retrieving SENDER data ******************************* END
							
							
							//*********************** Call BaaS proxy for retrieving RECEIVER data ******************************* START
							
							$baas_index_receiver = rand($min_baas_index,$max_baas_index);	//generate random number between min_baas_index to min_baas_index

							//below logic avoids sender and receiver to be the same
							while ($baas_index_receiver == $baas_index_sender) {			//go in loop if sender and receiver random numbers were same
								$baas_index_receiver = rand($min_baas_index,$max_baas_index);
								$random_number_sequence += 1;
								if ($random_number_sequence > 5) {		//break to avoid infinite loop
									break;
								}
							}
							
							$updated_baas_payload_receiver = str_replace($baas_id_string, $baas_index_receiver, $baas_payload);

							$baas_options_receiver = array(
							  'method' => 'POST',
							  'data' => $updated_baas_payload_receiver,
							  'headers' => array('Content-Type' => 'application/json'),
							  'timeout' => 30,
							);
														
							$baas_response_receiver = drupal_http_request($baas_url, $baas_options_receiver);
							$parsed_baas_response_receiver = str_replace("tempuser_","receiver",$baas_response_receiver->data);	
							
							//*********************** Call BaaS proxy for retrieving RECEIVER data ******************************* END
							
							//copy the sender and receiver data into session variables
							$_SESSION['baasresponse']['sender'] = $parsed_baas_response_sender;
							$_SESSION['baasresponse']['receiver'] = $parsed_baas_response_receiver;
						}
						else {
							//error handling
							$error = json_decode($baas_payload_node->data,true);
							drupal_set_message(t($error['Error']), 'error');
						}
					//}
				}
								
				//if (arg(1) == 'agentconnect_send' && $soap_operation_name == 'sendValidation') {			//for sendValidation				
				if ((arg(1) == 'agentconnect_send' || (arg(1) == 'form_free' && arg(3) == 'usatomexformfreesend')) && $soap_operation_name == 'sendValidation' && (arg(3) != 'adustomex' && arg(3) != 'hdsuktodom')) {			//for sendValidation. Added condition for FormFree Send scenario
					$parsed_baas_response_sender_array = json_decode($_SESSION['baasresponse']['sender'],true);
					$parsed_baas_response_receiver_array = json_decode($_SESSION['baasresponse']['receiver'],true);
					$soap_payload_substr = '%getfrombaas%';
					
					//Logic to add formFreeStaging, primaryReceiptLanguage, secondaryReceiptLanguage fields to the $parsed_baas_response_sender_array
					$defaultValuesObject = $soap_operations_fields_node->field_default[LANGUAGE_NONE][0]['value'];
					$defaultValuesArray = json_decode($defaultValuesObject,true);
					
					//using the logic below to plug-in default values from BaaS into defaultValues string
					foreach ($defaultValuesArray as $key => $value) {
						if ($defaultValuesArray[$key] == $soap_payload_substr) {
							if (strpos($key,"sender") !== false ) {
								$defaultValuesArray[$key] = $parsed_baas_response_sender_array[$key];
							}
							elseif (strpos($key,"receiver") !== false) {
								$defaultValuesArray[$key] = $parsed_baas_response_receiver_array[$key];
							}
							else {
								//invalid scenario
							}
						}
					}					
					$defaultValues = $defaultValuesArray;
					// for retrieving customer information from BaaS - END ***********************************
				}
				elseif (((arg(1) == 'agentconnect_receive' || (arg(1) == 'form_free' && arg(3) == 'recusatomexformfreereceive')) && $soap_operation_name == 'receiveValidation')) {			//for receiveValidation. Added condition for FormFree Receive scenario
					//elseif (arg(1) == 'agentconnect_receive' && $soap_operation_name == 'receiveValidation') {			//for receiveValidation
					
					$parsed_baas_response_sender_array = json_decode($_SESSION['baasresponse']['sender'],true);
					$parsed_baas_response_receiver_array = json_decode($_SESSION['baasresponse']['receiver'],true);
					$soap_payload_substr = '%getfrombaas%';
					
					//Logic to add formFreeStaging, primaryReceiptLanguage, secondaryReceiptLanguage fields to the $parsed_baas_response_sender_array
					$defaultValuesObject = $soap_operations_fields_node->field_default[LANGUAGE_NONE][0]['value'];
					$defaultValuesArray = json_decode($defaultValuesObject,true);
					
					//using the logic below to plug-in default values from BaaS into defaultValues string
					foreach ($defaultValuesArray as $key => $value) {
						if ($defaultValuesArray[$key] == $soap_payload_substr) {
							if (strpos($key,"sender") !== false ) {
								$defaultValuesArray[$key] = $parsed_baas_response_sender_array[$key];
							}
							elseif (strpos($key,"receiver") !== false) {
								$defaultValuesArray[$key] = $parsed_baas_response_receiver_array[$key];
							}
							else {
								//invalid scenario
							}
						}
					}					
					$defaultValues = $defaultValuesArray;
				}
				else {			//standard logic for pulling data from the content -- Gurpreet
					$defaultValuesObject = $soap_operations_fields_node->field_default[LANGUAGE_NONE][0]['value'];
					$defaultValues = json_decode($defaultValuesObject,true);
				} 				//standard logic for pulling data from the content -- Gurpreet
				
				$excludeFields = explode(",", $soap_operations_fields_node->field_excludefields[LANGUAGE_NONE][0]['value']);
				if($soap_operations_fields_node->field_sessionid[LANGUAGE_NONE][0]['value']){
					$session_var = $soap_operations_fields_node->field_sessionid['value'];
				}
			}
			//BLOCK2: Retreive include exclude fields for the current operation - End (Gurpreet)
		}
      }
    }
  } 
  
  //BLOCK3: Get the attributes from the web service description - Start (Gurpreet)
    foreach ($operations as $op=> $opinfo) {
    if ($opinfo['label'] === $soap_operation_name)
    {
      $operation = $operations[$op];
      //$label = $opinfo['label'];
      $result = $opinfo['result'];
      $parameter = $opinfo['parameter'];
      array_push($certs, $opinfo);
      }
    } 
    //BLOCK3: Get the attributes from the web service description - End (Gurpreet)
    //BLOCK1: Retreive Operations for a API Category - End (Gurpreet)
 
  //get default values from service entity
  //$defaultValues = $operation['certification'][$certification]['default'];    //commented - Gurpreet
  //$_SESSION = NULL;

  //parse xml response from session if available
  if ($_SESSION['lastResponse']) {
    $xml = simplexml_load_string($_SESSION['lastResponse']);
    $responseArray = $xml->children('soapenv', TRUE)->Body->children('ac', TRUE);
    $json = json_encode($responseArray);
  $responseArray = json_decode($json, TRUE);
  
    //grab the last response and cache
    $responseID = array_shift(array_keys($responseArray));
  
    if (isset($_SESSION['cached_responses'])) {
      if (empty($_SESSION['cached_responses'])) {
        //cache is empty but set
        $_SESSION['cached_responses'] = $responseArray;
      }
      elseif (isset($_SESSION['cached_responses'][$responseID])) {
        //response has been cached previously, update cache with new response
        unset($_SESSION['cached_responses'][$responseID]);
        $_SESSION['cached_responses'] = array_merge($_SESSION['cached_responses'], $responseArray);
      }
      else {
        //some response has been cached before, add new response type to cache
        if (isset($responseArray)) {
          $_SESSION['cached_responses'] = array_merge($_SESSION['cached_responses'], $responseArray);
        }
      }
    }
    else {
      //first cached response
      $_SESSION['cached_responses'] = $responseArray;
    }
  }
  
    //parse xml request from session if available
    if ($_SESSION['lastRequest']) {
      $xml_req = simplexml_load_string($_SESSION['lastRequest']);
      $requestArray = $xml_req->children('SOAP-ENV', TRUE)->Body->children('ns1', TRUE);
      $json = json_encode($requestArray);
      $requestArray = json_decode($json, TRUE);

      //grab the last response and cache
      $requestID = array_shift(array_keys($$requestArray));

      if (isset($_SESSION['cached_requests'])) {
        if (empty($_SESSION['cached_requests'])) {
          //cache is empty but set
          $_SESSION['cached_requests'] = $requestArray;
        }
        elseif (isset($_SESSION['cached_requests'][$requestID])) {
          //response has been cached previously, update cache with new response
          unset($_SESSION['cached_requests'][$requestID]);
          $_SESSION['cached_requests'] = array_merge($_SESSION['cached_requests'], $requestArray);
        }
        else {
          //some response has been cached before, add new response type to cache
          if (isset($requestArray)) {
            $_SESSION['cached_requests'] = array_merge($_SESSION['cached_requests'], $requestArray);
          }
        }
      }
      else {
        //first cached response
        $_SESSION['cached_requests'] = $requestArray;
      }
  }
  
  //kill last response & request
  //$_SESSION['lastResponse'] = NULL;
  //$_SESSION['lastRequest'] = NULL;

  //dpm($_SESSION,"SESSION VARIABLES");

  //add values from session to default values to be be loaded on form
  $cached_responses = array_merge($_SESSION['cached_responses'],$_SESSION['cached_requests']);
  
  if($includeFields)
  {
    $incReq = array();
    foreach ($cached_responses as $cache_resp => $cache_info) 
  {
      foreach($includeFields as $incField => $incFieldInfos)
      {
        foreach($incFieldInfos as $incFieldInfo => $incFieldMappings)
          {
            if ($incField === $cache_resp)
            {
              //$test = jsonPath($cache_info, "$.feeInfo.deliveryOption")[0];
              $incReq[$incFieldMappings] = jsonPath($cache_info, "$." . $incFieldInfo)[0];
            }
          }
       }
    }
    if (isset($defaultValues)) {
      $defaultValues = array_merge($defaultValues, $incReq);
    }
    else{
      $defaultValues = $incReq;
    }
  }
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
        
//Execute ref no logic only for agentconnect_receive
  if((arg(1) == 'agentconnect_receive' && (int)$certification_step == 0) || !empty($form_state['values']['gen-reference-soap-submit']) || (arg(1) == 'form_free' && arg(3) == 'recusatomexformfreereceive' && (int)$certification_step == 0) || (arg(1) == 'amend' && (int)$certification_step == 0) || (arg(1) == 'send_reversal' && (int)$certification_step == 0)){
    $soap_payload_key = arg(3);
    $soap_payload_node =  node_load_by_title($soap_payload_key, 'acrspayload');
	$ref_Num_Request = $soap_operation_name . "Request";
	$_SESSION['acoperations']['operations']['request'] = $ref_Num_Request;
	
    if ($soap_payload_node) {
      // Load the sample payload
        $soap_payload_default = $soap_payload_node->body[LANGUAGE_NONE][0]['value'];
		
		// **********  add logic to replace the stub string with sender/receiver values ********* START
		$soap_payload_substr = '%getfrombaas%';				//defined above
		$soap_payload_len = strlen($soap_payload_substr);		//defined above					
		$soap_payload_substr_pos = strpos($soap_payload_default, $soap_payload_substr);		//first occurance of substring
		
		//get the sender/receiver BaaS data from session variable
		$parsed_baas_response_sender_array = json_decode($_SESSION['baasresponse']['sender'],true);
		$parsed_baas_response_receiver_array = json_decode($_SESSION['baasresponse']['receiver'],true);
		
		while ($soap_payload_substr_pos !== false) {
			$temp_str_left = substr($soap_payload_default,0,$soap_payload_substr_pos);		//find string left of the soap_payload_substr
			$temp_str_right = substr($soap_payload_default,$soap_payload_substr_pos + $soap_payload_len);	//find string right of the soap_payload_substr
			
			//xml tag start/end characters
			$from = "<";	
			$to = ">";
			
			//search from last (using strrpos) to find the last xml tag in the left string
			$tmp_str_tag = substr($temp_str_left,strrpos($temp_str_left,$from)+strlen($from),-1);
			$tmp_str_tag_array = explode(":",$tmp_str_tag);		//find the xml tag name
			$key = $tmp_str_tag_array[1];
			
			//get value for the tag from BaaS content
			if (strpos($key,"sender") !== false ) {
				$value = $parsed_baas_response_sender_array[$key];
			}
			elseif (strpos($key,"receiver") !== false) {
				$value = $parsed_baas_response_receiver_array[$key];
			}
			else {
				//invalid scenario
			}
			
			$soap_payload_default =  $temp_str_left . $value . $temp_str_right;
			$soap_payload_substr_pos = strpos($soap_payload_default, $soap_payload_substr);		//find next occurance of substring
		}			
		
		$soap_payload = $soap_payload_default;		// assigned the final value to soap_payload
    }

    $form['gen-reference-soap-payload'] = array(
          '#title' => 'Sample Payload',
          '#type' => 'textarea',
          '#default_value' => $soap_payload,
          '#prefix' => '<div class="soap_body_textarea">',
          '#suffix' => '</div>', 
          '#attributes' => array('id' => 'edit-ref'),
    );
  
    $form['gen-reference-soap-submit'] = array(
      '#type' => 'button',
      '#value' => 'Generate Reference Number',
      '#name'  => 'gen-reference-soap-sub',
      '#prefix' => '<div class="get_reference_number_button">',
      '#suffix' => '</div>', 
      '#ajax' => array(
        'callback' => 'get_reference_number',
        'wrapper' => 'replace_reference_number_value',
        'method' => 'replace',
        'effect' => 'fade',
      ),
    );
  }

  /* hide this for now
    $form['uri'] = array(
      '#title' => 'Service URI',
      '#type' => 'textfield',
      '#disabled' => TRUE,
      '#value' => $service->url,
    );
  */
  
  $form['parameters'] = array(
    '#title' => 'Parameters',
    '#type' => 'container',
    '#tree' => TRUE,
    '#value' => t("Enter the parameters to the !operation_label service here. It's up to you to get the data types right. No type validation is done at this end, as it's a debugger to let you throw various errors at the web service and see how it responds.", $strings),
  );

  //custom css for form parameters
  $form['parameters']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css';   
  // Deal with complex types.
  // Each complex type may require its own mini-form for data entry,
  // and these may be nested.
  $datatypes = $service->datatypes;
  foreach ($operation['parameter'] as $param => $info) {
      $form['parameters'][$param] = soap_client_certify_data($param, $info['type'], @$info['description'], @$form_state['values']['parameters'], $datatypes, @$info['required'], $excludeFields, $defaultValues);
  }
  
/*  replacing 'referenceNumberRequest' with $ref_Num_Request - Gurpreet
  $form['parameters']['referenceNumberRequest']['referenceNumber']['#prefix'] = '<div id="replace_reference_number_value">';
  $form['parameters']['referenceNumberRequest']['referenceNumber']['#suffix'] =  '</div>';
*/
  $form['parameters'][$ref_Num_Request]['referenceNumber']['#prefix'] = '<div id="replace_reference_number_value">';
  $form['parameters'][$ref_Num_Request]['referenceNumber']['#suffix'] =  '</div>';
  
    // An AJAX request calls the form builder function for every change.
    // We can change how we build the form based on $form_state.
    if (!empty($form_state['values']['gen-reference-soap-submit'])) {
    /*  replacing 'referenceNumberRequest' with $ref_Num_Request - Gurpreet
      unset($form_state['input']['parameters']['referenceNumberRequest']['referenceNumber']);
    */
    unset($form_state['input']['parameters'][$ref_Num_Request]['referenceNumber']);
     
    // Make a call to API Proxy to generate reference number
      // Make the call to ping..

      $payload = $form_state['values']['gen-reference-soap-payload'];

      $options = array(
          'method' => 'POST',
          'data' => $payload,
          'timeout' => 30,
          'headers' => array('Content-Type' => 'application/xml', 'SOAPAction' => 'urn:AgentConnect1305#commitTransaction'),
      );
    
      //call to apigee mashupsendcommit proxy
		$mashup_payload_key = "mashupproxyurl";
		$mashup_payload_node =  node_load_by_title($mashup_payload_key, 'acconfig');
		if ($mashup_payload_node) {
			$mashup_payload = $mashup_payload_node->body[LANGUAGE_NONE][0]['value'];
			$mashup_payload_array = json_decode($mashup_payload,true);
			$mashup_url_string = $mashup_payload_array['mashup_proxy_url'];	
		}
		else {
			//error handling
			$error = json_decode($mashup_payload_node->data,true);
			drupal_set_message(t($error['Error']), 'error');
		}
		
		$result = drupal_http_request($mashup_url_string, $options);		//getting the proxy URL from the Content			
		//mashup_url_string = 'https://moneygram-ext.apigee.net/v1/mashupsendcommit/commitTrans'

      // Extract ref number from above result using json parse      
      $ref_result = json_decode($result->data,true);
  
      //Error handling
      $error = json_decode($result->data,true);
      drupal_set_message(t($error['Error']), 'error');
    
    /*  replacing 'referenceNumberRequest' with $ref_Num_Request - Gurpreet
      $form['parameters']['referenceNumberRequest']['referenceNumber']['#default_value'] =$ref_result['ReferenceNumber'];
    */
    $form['parameters'][$ref_Num_Request]['referenceNumber']['#default_value'] = $ref_result['ReferenceNumber'];

      //this is used for debugging
    /* $form['parameters']['referenceNumberRequest']['referenceNumber']['#description'] = "<pre>" . print_r($result, 1) . "</pre>";
     */
    }
  
  $form['execute'] = array(
    '#type' => 'submit',
    '#value' => 'Execute Request',
    '#ajax' => array(
      'event' => 'click',
      'callback' => 'wsclient_tester_prepare_request_callback',
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
  
  //todo: check if this is last element in certs array
  //if(arg(1) == 'agentconnect_receive' && (int)$certification_step < ($operationsCount-1)){  --  had issues, was only showing up for agentconnect_receive
  if((int)$certification_step < ($operationsCount-1)){
    $intstep = (int)$certification_step;
    $nextstep = $intstep + 1;
    //$nextstep = 1;
    $proceedUrl = "/soap/" . $service->name . "/certify/" . $certification . "/" . $nextstep;

    //add some markup here
    $form['resp_spacer'] = array(
    '#markup' =>   '<br><a role="button" class="btn btn-primary" border="square" href="' . $proceedUrl . '" style="padding:15px 10px !important;"><i class="fa fa-files-o"></i>Proceed to Next API</button></a><br/><br/>',
    );
  }
  elseif ((int)$certification_step = ($operationsCount-1)) {
  $form['resp_spacer'] = array(
    '#markup' =>   '<br><h4>This is the end of this Sample Transaction</h4><br/><br/>',
    );
    
    unset($_SESSION['acoperations']['operations']['key']);  //added this to unset the key on the load of last Sample Transaction API page
	unset($_SESSION['baasresponse']['sender']);	//added this to unset the sender data from BaaS
	unset($_SESSION['baasresponse']['receiver']);	//added this to unset the receiver data from BaaS
  }

  /* hide this for now
    $form['response'] = array(
      '#type' => 'fieldset',
      '#title' => 'Response Definition',
    );

    //display the result parameters
    $form['response']['results'] = array(
      '#title' => 'result',
      '#type' => 'container',
      '#tree' => TRUE,
    );
    $form['response']['results']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css'; 

    // Deal with complex types.
    // Each complex type may require its own mini-form for data entry,
    // and these may be nested.
    

    $datatypes = $service->datatypes;
      $form['response']['results']['definition'] = soap_client_response_data($operation['result'], $operation['result']['type'], @$resinfo['description'], @$form_state['values']['parameters'], $datatypes, @$resinfo['required']);
  */

  return $form;
}

/**
 * FAPI form used to display the properties of a datatype
 */
function soap_client_type_viewer($form, $form_state, $service, $type){
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
    $form['types']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css';

    /* hide this for now
    $form['properties'] = array(
      '#tree' => TRUE,
      '#element_validate' => array('wsclient_ui_validate_parameters'),
      //'#theme' => 'wsclient_ui_property_form',
      '#title' => t('Properties'),
      '#description' => t('Each property will be shown for the data type below.'),
    );
    $types = soapclient_ui_types();
    if (isset($type['property info'])) {
      foreach ($type['property info'] as $name => $info) {
        //$form['properties']['items'][$name] = _soapclient_ui_property_row($service, $types, $name, $info);
        $form['properties']['items'][$name] = _soapclient_ui_property_row_new($name);
      }
    }
    */

  return $form;
}

/**
 * FAPI form used to display all operations and datatypes of a service
 */
function soap_client_operation_viewer($form, $form_state, $service) {
  //debugging ops and service
  $ops = $service->operations;
  firep($ops);
  firep($service);

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

    /* hide this for now
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => 'Service Name:',
      '#default_value' => $service->label,
      '#disabled' => TRUE,
      '#weight' => 10,
    );
    */

      $form['header-title'] = array(
        '#markup' => '<h2>' . $service->label . '</h2><br>',
        );

    /* hide this for now
    $form['url'] = array(
      '#type' => 'textfield',
      '#title' => 'URL',
      '#default_value' => $service->url,
      '#disabled' => TRUE,
      '#element_validate' => array('wsclient_ui_element_url_validate'),
      '#weight' => 30,
    );
    */

   $rows = array();
      $operations = soap_client_label_sort($service->operations);
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
      $form['operations']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css';



      // Data types of the web service in a table
      $rows = array();
      $datatypes = soap_client_label_sort($service->datatypes);
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
      $form['datatypes']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css';

    return $form;
}

/**
 * FAPI form used to display all services that are managed
 */
function soap_client_services_viewer($form, $form_state) {
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
function wsclient_tester_prepare_request_callback($form, $form_state) {
  firep($form_state);
  $service = $form_state['build_info']['args'][0];

  if ($form_state['build_info']['form_id'] === 'soap_client_certify_operation_invoke'){
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
  //session variable set in soap_client_certify_operation_invoke function
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

  //firep($operation);
  //firep($service);

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
    $element['request']['packet']['#markup'] = htmlspecialchars(wsclient_tester_prettify_xml($service->endpoint()->client()->__getLastRequest()));
    $_SESSION['lastRequest'] = $service->endpoint()->client()->__getLastRequest();

    $element['response']['header']['#markup'] = $service->endpoint()->client()->__getLastResponseHeaders();
    $element['response']['packet']['#markup'] = htmlspecialchars(wsclient_tester_prettify_xml($service->endpoint()->client()->__getLastResponse()));
    $_SESSION['lastResponse'] = $service->endpoint()->client()->__getLastResponse();
 }

  /* hide this for now
  if (module_exists('devel')) {
    $element['request']['data']['#markup'] = kpr($args, 1);
    $element['response']['data']['#markup'] = kpr($response, 1);
   }
   */

  $element['#value'] = t("Ran at %time, took %duration to execute", array(
    '%time' => time(),
    '%duration' => $timer_duration,
  ));
  
  return $element;
}

/**
 * Indent and format XML for display.
 */
function wsclient_tester_prettify_xml($xml) {
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
function soap_client_label_sort($data_info, $label_key = 'label') {
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
function soap_client_request_data($label, $type, $description, $data, $datatypes, $required) {
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
          firep("$field_id - true - $one");
          $element[$field_id] = soap_client_request_data($field_id, $field_info['type'], @$field_info['description'], $attributes[$field_id], $datatypes, @$field_info['required']);
        }
      elseif($field_id == "timeStamp")
        {
          $dt = new DateTime();
          $dt->setTimeZone(new DateTimeZone('UTC'));
          $timeStamp = $dt->format('c');
          $element[$field_id] = soap_client_request_data($field_id, $field_info['type'], @$field_info['description'], $timeStamp, $datatypes, @$field_info['required']);
        }
      else
        {
          $two = $attributes[$field_id];
          firep("$field_id - false - $two ");
          $element[$field_id] = soap_client_request_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes, @$field_info['required']);
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
      $element[$field_id] = soap_client_request_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes);
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

function soap_client_certify_data($label, $type, $description, $data, $datatypes, $required, $excludeFields, $defaultValues) {

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
        //$one = $attributes[$field_id];
        //firep("$field_id - true - $one");
        $element[$field_id] = soap_client_certify_data($field_id, $field_info['type'], @$field_info['description'], $attributes[$field_id], $datatypes, @$field_info['required'], $excludeFields, $defaultValues);
      }
      elseif($field_id == "timeStamp")
      {
        $dt = new DateTime();
        $dt->setTimeZone(new DateTimeZone('UTC'));
        $timeStamp = $dt->format('c');
        $element[$field_id] = soap_client_certify_data($field_id, $field_info['type'], @$field_info['description'], $timeStamp, $datatypes, @$field_info['required'], $excludeFields, $defaultValues);
      }
      else
      {
        //$two = $attributes[$field_id];
        //firep("$field_id - false - $two ");
       $element[$field_id] = soap_client_certify_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes, @$field_info['required'], $excludeFields, $defaultValues);
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
      $element[$field_id] = soap_client_certify_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes, @$field_id['required'], $excludeFields, $defaultValues);
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
function soap_client_response_data($label, $type, $description, $data, $datatypes) {
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
      $element[$field_id] = soap_client_response_data($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes);
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
      $element[$field_id] = soap_client_response_data($field_id, $actual_type, NULL, $data[$field_id], $datatypes);
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

function node_load_by_title($title, $node_type) {
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