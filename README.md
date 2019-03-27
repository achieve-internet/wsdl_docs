# WSDL Docs
SOAP API documentation via WSDL imports.  Converts operations from the WSDL to nodes.

## SETUP
* This module creates nodes from Operations in a WSDL and allows sending of SOAP requests for those operations.
* Ensure SOAP is installed on your server.
* Enable Required modules
    * wsdl_docs
    * wsclient
    * wsclient_soap
    * entityreference
    * pathauto (If you want SEO friendly URLs on operation nodes)
    * services and rest_server (If you want to create a REST endpoint for CRUD operations on WSDL docs)
* Enable Features
    * wsdl_docs_features
* Permissions: There is a 'WSDL Docs Operations using Services module' permission that is checked when making requests via REST. Make sure you add a user that has this permission, and use that user's login credentials when making REST requests.

## USAGE
* Import WSDL via Drupal admin UI
    * /admin/content/wsdl_docs/add
* View created operation nodes
    * /admin/content, filter by Type: 'WSDL Docs Operation'
* Update existing service/operations via Drupal admin UI
    * Click edit on admin/content/wsdl_docs for the service you want to update the operations on.
    * Click save.

### CUD WSDL Docs via REST (thanks to Services module)
1. Make sure the services module is enabled
1. At admin/structure/services/add add your service, select REST as the Server, and save. Select HTTP basic authentication for the authentication.
1. Once you've added your service, click on the Edit Resources configuration option at admin/structure/services for your service
1. Under the WSDL resource, select the operations you want to make available (CUD available only right now) and save

### cURL commands for the operations enabled by this module:
Note: Use HTTP Basic Auth for these APIs (Username/password is the credentials for the account with 'WSDL Docs Operations using Services module' permissions setup on DevPortal)*
#### Create a WSDL Doc Service
`curl -X POST [your-website-url]/[your-service-path-to-endpoint]/wsdl \
-H 'Content-Type: application/json' \
-H 'Authorization: Basic c2VydmljZXMtYWRtaW46cGFzc3dvcmQ=' \
-d '{"name": "test8", "url": "http://www.thomas-bayer.com/axis2/services/BLZService?wsdl"}'`
#### Update a WSDL Doc Service's Operations by URL of WSDL resource
`curl -X PUT [your-website-url]/[your-service-path-to-endpoint]/wsdl/[wsdl-doc-name] \
-H 'Authorization: Basic c2VydmljZXMtYWRtaW46cGFzc3dvcmQ=' \
-H 'Content-Type: application/json' \
-d '{"url": "https://graphical.weather.gov/xml/DWMLgen/wsdl/ndfdXML.wsdl"}'`
#### Delete a WSDL Doc Service and its Operations
`curl -X DELETE [your-website-url]/[your-service-path-to-endpoint]/wsdl/[wsdl doc name] \
-H 'Authorization: Basic c2VydmljZXMtYWRtaW46cGFzc3dvcmQ='`
#### Import a WSDL to an Existing Service by WSDL file
`curl -X POST [your-website-url]/[your-service-path-to-endpoint]/wsdl/[wsdl doc name]/import \
-H 'Authorization: Basic c2VydmljZXMtYWRtaW46cGFzc3dvcmQ=' \
-H 'content-type: multipart/form-data' \
-F 'soap_api_definition=@[path-to-wsdl]'`

* View updated operations (see updated timestamp on node)
    * /admin/content
* View Operations and services from front end
    * /soap_apis
* Run Operation
    * From /soap_apis click select a service.
    * All of the service's operations will be displayed via a table generated using Views.
    * Select an operation.
    * Executing that request will display the following:
        * the request that is being made (in raw form with headers and payload)
        * the response that came back, also displaying
        * the response headers and response data or message if any.
        * If you have devel.module enabled, a rendered dump of the data structure (PHP objects constructed by the wsclient) will also be displayed.

## Sample WSDLs for testing (taken from Apigee Edge SOAP proxy demo)
* http://s3.amazonaws.com/ec2-downloads/ec2.wsdl
* https://www.paypalobjects.com/wsdl/PayPalSvc.wsdl
* http://webservices.flightexplorer.com/FastTrack.asmx?wsdl
* http://doc.s3.amazonaws.com/2006-03-01/AmazonS3.wsdl
* http://www.thomas-bayer.com/axis2/services/BLZService?wsdl (useful to test only 1 operation)
* https://graphical.weather.gov/xml/DWMLgen/wsdl/ndfdXML.wsdl

## INFO

Validation is not done by this UI, so it's up to you to enter the correct basic 
data types (e.g., date in a date field, int in an integer field)

It was designed to handle advanced, nested complex types as data structures 
that can be both submitted and retrieved. If wsclient can manage the data, we 
should be able to expose it.

## HISTORY

This module started as a request for a standalone form to test web services before digging into rules http://drupal.org/node/1812504 -> https://www.drupal.org/node/1929778 it then became absorbed into WSClient https://www.drupal.org/project/wsclient. Later on it was customized to focus on testing SOAP webservices https://github.com/apickelsimer/soap_client_portal it's latest iteration is wsdl_docs.

This module was originally developed by Alex Borsody (AlexBorsody) - https://www.drupal.org/u/alexborsody.
Clean-up and additional configuration work contributed by Kristin Brinner (kbrinner) - https://www.drupal.org/u/kbrinner.

## MAINTAINERS

### Current maintainers:
* Alex Borsody (AlexBorsody) - https://www.drupal.org/u/alexborsody
* Kristin Brinner (kbrinner) - https://www.drupal.org/u/kbrinner

### This project has been sponsored by:
* Achieve Internet - https://www.drupal.org/achieve-internet