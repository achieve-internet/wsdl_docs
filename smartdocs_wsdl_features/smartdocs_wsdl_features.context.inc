<?php
/**
 * @file
 * smartdocs_wsdl_features.context.inc
 */

/**
 * Implements hook_context_default_contexts().
 */
function smartdocs_wsdl_features_context_default_contexts() {
  $export = array();

  $context = new stdClass();
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'related_operations';
  $context->description = 'Displays all of a service\'s operations';
  $context->tag = '';
  $context->conditions = array(
    'node' => array(
      'values' => array(
        'operations' => 'operations',
      ),
      'options' => array(
        'node_form' => '1',
      ),
    ),
  );
  $context->reactions = array(
    'block' => array(
      'blocks' => array(
        'views-operations-api_explorer' => array(
          'module' => 'views',
          'delta' => 'operations-api_explorer',
          'region' => 'sidebar_first',
          'weight' => '-10',
        ),
      ),
    ),
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Displays all of a service\'s operations');
  $export['related_operations'] = $context;

  return $export;
}
