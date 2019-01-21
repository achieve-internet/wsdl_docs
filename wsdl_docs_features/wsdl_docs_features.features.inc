<?php
/**
 * @file
 * wsdl_docs_features.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function wsdl_docs_features_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "context" && $api == "context") {
    return array("version" => "3");
  }
}

/**
 * Implements hook_views_api().
 */
function wsdl_docs_features_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function wsdl_docs_features_node_info() {
  $items = array(
    'operations' => array(
      'name' => t('WSDL Docs Operation'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
