<?php

/**
 * @file Site dna storage class
 */

// If the storage format changes, make sure to increase!
define('DNA_VERSION', '0.1');

require_once 'dna_output_interface.inc';

/**
 * Retrieves and stores the site settings
 */
class dna_settings implements dna_output {
  /**
   * @var modules settings
   */
  private $modules;

  /**
   * @var variable settings
   */
  private $variables;

  /**
   * @var enabled modules
   */
  private $enabled_modules;

  /**
   * @var timestamp
   */
  private $timestamp;


  /**
   * When constructed it pulls in all the modules
   *
   * @todo change it to ondemand later
   */
  function  __construct() {
    $this->pull_variables();
    $this->pull_modules();
    $this->pull_enabled_modules();
    $this->timestamp = time();
  }


  /**
   * Pulls version info about the enabled modules
   */
  private function pull_enabled_modules() {
    $query = db_query('SELECT name, weight, status, info FROM {system}');
    while ($row = db_fetch_object($query)) {
      $this->enabled_modules[$row->name]['info'] = unserialize($row->info);
      $this->enabled_modules[$row->name]['weight'] = $row->weight; 
      $this->enabled_modules[$row->name]['status'] = (bool)$row-status;
    }
  }


  /**
   * Get all the variables into an internal storage
   */
  private function pull_variables() {
    global $conf;
    $this->variables = $conf;
  }


  /**
   * Get all the modules settings into the internal storage
   */
  private function pull_modules() {
    module_load_include('inc', 'features', 'features.export');
    features_include();

    foreach (module_list() as $module) {
      $export_options = $module .'_features_export_options';
      $data = array();
      if (function_exists($export_options)) {
        $data = $export_options();
      }

      $export = array('dependencies' => module_list());
      $export_render = $module .'_features_export_render';
      if (function_exists($export_render)) {
        $fingerprint[$module]['#default'] = $export_render($module, $data, $export);
      }
    }

    $stub = array();
    foreach (features_get_components(TRUE) as $component => $info) {
      if ($options = features_invoke($component, 'features_export_options')) {
        $stub[$component] = array_keys($options);
      }
    }
    $export = features_populate($stub, 'test');
    $rendered_hooks = features_export_render_hooks($export, 'test', TRUE);
    $this->modules = $this->eval_fingerprint(array_merge_recursive($rendered_hooks, $fingerprint));
  }


  /**
   * Evals the elements of the fingerprint to render it as an array
   */
  private function eval_fingerprint(Array &$fingerprint) {
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
   * Outputs the settings as string
   */
  public function get_output() {
    //return var_export(array('modules' => $this->modules, 'variables' => $this->variables), TRUE);
    //var_dump($this->enabled_modules);
    dd($this);
    return '';
  }


  /**
   * Returns the module info in an array
   * This has to be consistent across versions
   *
   * @param module name
   * @return Mixed Array of the module info, or FALSE if not found
   */
  public function module_info(String $module) {
    if (!isset($this->module_info[$module]))
      return FALSE;

    $ret = array();
    return array_merge($this->module_info[$module], $this->modules[$module]);
  }

}

