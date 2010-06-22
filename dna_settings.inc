<?php

/**
 * @file Site dna storage class
 */

require_once 'dna_output_interface.inc';

/**
 * Retrieves and stores the site settings
 */
class dna_settings implements dna_output {
  private $modules;
  private $variables;

  function  __construct() {
    $this->pull_variables();
    $this->pull_modules();
  }


  /**
   * Get all the variables into an internal storage
   */
  private function pull_variables() {
    global $conf;
    $this->variables = $conf;
  }


  /**
   * Get all the modules settings into an internal storage
   */
  private function pull_modules() {
    module_load_include('inc', 'features', 'features.export');
    features_include();
    $export = array();
    $fingerprint = array();
    foreach (module_list() as $module) {
      $export_options = $module .'_features_export_options';
      $data = array();
      if (function_exists($export_options)) {
        $data = $export_options();
      }

      $export = array('dependencies' => module_list());
      $export_render = $module .'_features_export_render';
      if (function_exists($export_render)) {
        $fingerprint[$module] = $export_render($module, $data, $export);
      }
    }

    $this->modules = $this->eval_fingerprint($fingerprint);
  }


  /**
   * Evals the elements of the fingerprint to render it as an array
   */
  private function eval_fingerprint(&$fingerprint) {
    foreach ($fingerprint as $key => &$element) {
      if (is_string($element)) {
        $fingerprint[$key] = @eval($element);
      } else if (is_array($element)) {
        $this->eval_fingerprint($element);
      }
    }
    return $fingerprint;
  }


  /**
   * Returns a module exportable settings
   *
   * @param $module_name String
   */
  public function get_module($module_name) {
    if (isset($this->modules[$module_name])) {
      return $this->modules[$module_name];
    }

    return FALSE;
  }


  /**
   * Outputs the settings as string
   */
  public function get_output() {
    return var_export(array('modules' => $this->modules, 'variables' => $this->variables), TRUE);
  }

}
