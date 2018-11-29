<?php
/**
 * @file
 * smartdocs_wsdl_features.features.inc
 */

/**
 * Implements hook_views_api().
 */
function smartdocs_wsdl_features_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function smartdocs_wsdl_features_node_info() {
  $items = array(
    'operations' => array(
      'name' => t('WSDL SmartDocs Operation'),
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
