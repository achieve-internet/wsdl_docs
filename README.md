# WSDL Docs
## SOAP API documentation via WSDL imports.  
This module creates nodes from Operations in a WSDL and allows sending of SOAP requests for those operations.

## SETUP 
* Ensure SOAP is installed on your server.
* Enable Required modules
    * wsdl_docs
    * wsclient
    * wsclient_ui
    * wsclient_soap
    * entityreference
    * pathauto (If you want SEO friendly URLs on operation nodes)
    * services, services_basic_auth, and rest_server (If you want to create a REST endpoint for CRUD operations on WSDL docs)
* Enable Features
    * wsdl_docs_features
* Permissions: 
    * There is a 'Make REST requests for WSDL Docs via Services' permission that is checked when making requests via REST. Make sure you add a user that has this permission, and use that user's login credentials when making REST requests.
    * There is a 'Download WSDL and XSD documents as displayed on operation nodes' permission that allows users to download any WSDL files that were imported to a 'WSDL Doc' service. 

## USAGE
* Import WSDL via URL using the Drupal admin UI 
    * /admin/content/wsdl_docs/add
* View operation nodes
    * /admin/content, filter by Type: 'WSDL Docs Operation'
* Update existing service/operations via Drupal admin UI
    * Click edit on admin/content/wsdl_docs for the service you want to update the operations on.
    * Make desired changes. 
    * Click save.
* View updated operations (see updated timestamp on node)
    * /admin/content
* View Operations and services from front end
    * /soap_apis
* Run Operation
    * From /soap_apis select a service.
    * All of the service's operations will be displayed via a table generated using Views.
    * Select an operation.
    * Executing that request will display the following:
        * the request that is being made (in raw form with headers and payload)
        * the response that came back, also displaying
        * the response headers and response data or message if any.
        * If you have devel.module enabled, a rendered dump of the data structure (PHP objects constructed by the wsclient) will also be displayed.
* Download imported WSDL Docs on Operation nodes
    * If WSDL(s) are imported via file(s), the user will only be able to download the original WSDL files if they have been given the 'Download WSDL and XSD documents as displayed on operation nodes' permission.

## CUD WSDL Docs via REST (thanks to Services module)
### Configuring Services
1. Make sure the services, services_basic_auth, and rest_server modules are enabled
1. At admin/structure/services/add add your service, select REST as the Server, select HTTP basic authentication for the authentication, and save.
1. Once you've added your service, click on the Edit Resources configuration option at admin/structure/services for your service.
1. Under the WSDL resource, select the operations you want to make available (CUD available only right now) and save. Update allows you to update an existing service based on the URL of the WSDL resource. Import allows you to update an existing service based on uploaded file(s). 
1. Make sure you have a user with the permission 'WSDL Docs Operations using Services module'.
### Importing WSDL Docs via REST
WSDL/XSD files may be imported via URL or file:
1. First create your WSDL Doc Service (see Create a WSDL Doc Service below). This will add the service and can be viewed at admin/content/wsdl_docs.
1. Then import the .wsdl file itself via:
    1. File(s) (see Import a WSDL to an Existing Service by WSDL file below) OR
    1. URL of WSDL resource (see Update a WSDL Doc Service's Operations by URL of WSDL resource below)
### cURL commands for the operations enabled by this module:
Note: Use HTTP Basic Auth for these APIs. The username/password are the credentials for the account with 'WSDL Docs Operations using Services module' permissions setup on DevPortal)
#### Create a WSDL Doc Service
`curl -X POST [your-website-url]/[your-service-path-to-endpoint]/wsdl \
-H 'Content-Type: application/json' \
-H 'Authorization: Basic [authorization hash]' \
-d '{"name": "[name of new service]"}'`
#### Update a WSDL Doc Service's Operations by URL of WSDL resource
`curl -X PUT [your-website-url]/[your-service-path-to-endpoint]/wsdl/[wsdl-doc-machine-name] \
-H 'Authorization: Basic [authorization hash]' \
-H 'Content-Type: application/json' \
-d '{"url": "[URL of WSDL/XSD file ie http://www.thomas-bayer.com/axis2/services/BLZService?wsdl]"}'`
#### Import a WSDL file(s) to an Existing Service by WSDL file
For importing multiple files, be sure the first file in the list is the main file that makes any references to additional files. The first file imported will be renamed [service-machine-name]-[service-id].[file extension (wsdl/xsd)] and saved to sites/default/private/wsdl_docs_files. If multiple files are imported, an archive of all of the files will be saved as [service-machine-name]-[service-id].zip.
`curl -X POST [your-website-url]/[your-service-path-to-endpoint]/wsdl/[wsdl-doc-machine-name]/import \
-H 'Authorization: Basic [authorization hash]' \
-H 'content-type: multipart/form-data' \
-F 'soap_api_definition[]=@[path-to-main-wsdl]'
-F 'soap_api_definition[]=@[path-to-other-wsdls]'`
#### Delete a WSDL Doc Service and its Operations
`curl -X DELETE [your-website-url]/[your-service-path-to-endpoint]/wsdl/[wsdl-doc-machine-name] \
-H 'Authorization: Basic [authorization hash]'`
## Sample WSDLs for testing (taken from Apigee Edge SOAP proxy demo)
* http://s3.amazonaws.com/ec2-downloads/ec2.wsdl
* https://www.paypalobjects.com/wsdl/PayPalSvc.wsdl
* http://webservices.flightexplorer.com/FastTrack.asmx?wsdl
* http://doc.s3.amazonaws.com/2006-03-01/AmazonS3.wsdl
* http://www.thomas-bayer.com/axis2/services/BLZService?wsdl (useful to test only 1 operation)
* https://graphical.weather.gov/xml/DWMLgen/wsdl/ndfdXML.wsdl

## INFO
Validation is not done by this UI, so it's up to you to enter the correct basic data types (e.g., date in a date field, int in an integer field)

It was designed to handle advanced, nested complex types as data structures that can be both submitted and retrieved. If wsclient can manage the data, we should be able to expose it.

## HISTORY
This module started as a request for a standalone form to test web services before digging into rules http://drupal.org/node/1812504 -> https://www.drupal.org/node/1929778 it then became absorbed into WSClient https://www.drupal.org/project/wsclient. Later on it was customized to focus on testing SOAP webservices https://github.com/apickelsimer/soap_client_portal. Its latest iteration is wsdl_docs.

This module was originally developed by Alex Borsody (AlexBorsody) - https://www.drupal.org/u/alexborsody.
Refactored to use the Services module by Kristin Brinner (kbrinner) - https://www.drupal.org/u/kbrinner.

## MAINTAINERS

### Current maintainers:
* Alex Borsody (AlexBorsody) - https://www.drupal.org/u/alexborsody
* Kristin Brinner (kbrinner) - https://www.drupal.org/u/kbrinner

### This project has been sponsored by:
* Achieve Internet - https://www.drupal.org/achieve-internet