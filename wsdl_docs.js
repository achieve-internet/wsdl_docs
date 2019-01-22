/**
 * @file
 * Custom js for WSDL docs module.
 */

(function ($) {
  'use strict';

  Drupal.behaviors.showTabs = {
    attach: function (context, settings) {
      $("#" + Drupal.settings.wsdl_docs.element_id).tabs();
    }
  };

}(jQuery));