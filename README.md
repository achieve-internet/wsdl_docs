# WSDL Docs
Module to provide basic documentation of SOAP Services, derived from WSDL 
files.

## Requirements
PHP must include the `ext-soap`, `ext-libxml`, and `ext-dom` modules.

## Installation
Download and enable the module following normal Drupal procedures.

If you would like a sidebar list on WSDL Docs Operation nodes showing other 
nodes that are derived from the same SOAP Service, you can place the available 
views block on your theme's sidebar region.

## Usage 

Create SOAP Service nodes with a url to a valid WSDL file. This will 
automatically generate WSDL Docs Operation nodes for each Operation found in 
the WSDL.

Views are used to generate the lists of WSDL Docs Operation nodes found on SOAP
Service nodes, and available for the sidebar (or other region) of WSDL Docs 
Nodes. Once the module is installed, these views blocks can be modified by an 
administrator as desired.  
