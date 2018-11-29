<?php

/**
 * Alter remote endpoint type definitions.
 *
 * @param $types
 *   The remote endpoint type definitions as returned from
 *   hook_wsclient_endpoint_types().
 *
 * @see hook_wsclient_endpoint_types()
 */
function smartdocs_wsdl_wsclient_endpoint_types_alter(&$types) {
  $types['soap']['class'] = 'WSClientSOAPEndpointUpload';
}

class WSClientSOAPEndpointUpload extends WSClientSOAPEndpoint {

  public function client() {
    if (!isset($this->client)) {
      $options['exceptions'] = TRUE;
      if (!empty($this->service->settings['options'])) {
        $options += $this->service->settings['options'];
      }
      if (!empty($this->url)) {
        $url = $this->url;
      }
      else {
        $url = 'private://smartdocs_wsdl_files/' . $this->service->label . '.wsdl';
        $options['cache_wsdl'] = WSDL_CACHE_NONE;
      }
      try {
        $this->client = new SOAPClient($url, $options);
      }
      catch (SoapFault $e) {
        throw new WSClientException('Error initializing SOAP client for service %name', array('%name' => $this->service->name));
      }
    }
    return $this->client;
  }

}
