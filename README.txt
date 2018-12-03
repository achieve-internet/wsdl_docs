=========================
SmartDocs WSDL
=========================

SmartDocs version for SOAP which converts operations from the WSDL to nodes.

Usage
-----
 
*This module creates nodes from Operations in a WSDL and allows sending of SOAP requests for those operations.

*Ensure SOAP is installed on your server.

*Enable Required modules
  smartdocs_wsdl
  wsclient
  wsclient_soap
  entityreference

*Enable Features
  smartdocs_wsdl_features

*Import
  Import WSDL
  /admin/content/smartdocs_wsdl/add

  *View created operation nodes
    /admin/content

*Update
  Update existing service
  Click edit on admin/content/smartdocs_wsdl for the service you want to update the operations on. 
  Click save. 

  *View updated operations (see updated timestamp on node)
    /admin/content

  *View Operations and services
    /soap_apis

*Run Operation
  From /soap_apis Click through to your operation 
  Executing that request will then display the request that is being made 
  (in raw form with headers and payload) and the response that came back, 
  also displaying the response headers and response data or message if any.
  If you have devel.module enabled, a rendered dump of the data structure 
  (PHP objects constructed by the wsclient) will also be displayed.

*REST Endpoint Import
  Configure HTTP basic auth credentials
  /admin/content/smartdocs_wsdl/basic_auth

  *Import from URL: This example will use Advanced REST Client to send the POST request to our REST endpoint.

  *TEST WSDL
  http://www.thomas-bayer.com/axis2/services/BLZService?wsdlde (this only has one operation so is good to use but can use any of the samples above).

  *Post to this url
    /smartdocs_wsdl_import
    Add parameters.
    url: The web address of the WSDL.
    name: The human readable label of the webservice that appears at /soap_apis.

  *View the created service at admin/content/smartdocs_wsdl

  *View created nodes
   /admin/content

*File Upload to Endpoint
  *This module has the ability to import viar /smartdocs_wsdl_import from a binary file upload, to test with Postman.

  *Select “file” tab and add "name" parameter.

  *Add headers, choose basic auth and add credentials, set content-type to application/xml.

  *On “body” tab choose binary and upload a sample WSDL.

  *Send
    If service does not exist you will see “CREATE {ID} SERVICE” and it will create the service.
    If it exists you will see “UPDATE {ID} SERVICE” and it will update the nodes associated with the service.

Sample WSDLs for testing (taken from Apigee Edge SOAP proxy demo)
http://s3.amazonaws.com/ec2-downloads/ec2.wsdl
https://www.paypalobjects.com/wsdl/PayPalSvc.wsdl
http://webservices.flightexplorer.com/FastTrack.asmx?wsdl
http://doc.s3.amazonaws.com/2006-03-01/AmazonS3.wsdl
http://www.thomas-bayer.com/axis2/services/BLZService?wsdl (useful to test only 1 operation)
https://graphical.weather.gov/xml/DWMLgen/wsdl/ndfdXML.wsdl

Info
----

Validation is not done by this UI, so it's up to you to enter the correct basic 
data types (e.g., date in a date field, int in an integer field)

It was designed to handle advanced, nested complex types as data structures 
that can be both submitted and retrieved. If wsclient can manage the data, we 
should be able to expose it.

History
-------

This module started as a request for a standalone form to test web services before digging into rules http://drupal.org/node/1812504 -> https://www.drupal.org/node/1929778 it then became absorbed into WSClient https://www.drupal.org/project/wsclient. Later on it was customized to focus on testing SOAP webservices https://github.com/apickelsimer/soap_client_portal it's latest iteration is SmartDocs_WSDL; Smartdocs for SOAP webservices. We hope to refactor it in the future and add improvements to make it on par with SmartDocs.
