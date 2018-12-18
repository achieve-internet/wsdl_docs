(function ($) {
  'use strict';

  Drupal.behaviors.showTabs = {
    attach: function (context, settings) {
      $("#" + Drupal.settings.smartdocs_wsdl.element_id).tabs();
    }
  };

}(jQuery));