<?php

/**
 * @file Site dna storage class
 */

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


  function  __construct() {
    $this->pull_variables();
    $this->pull_modules();
    $this->pull_enabled_modules();
  }


  /**
   * Pulls version info about the enabled modules
   */
  private function pull_enabled_modules() {
    $query = db_query('SELECT name, weight, info FROM {system}');
    while ($row = db_fetch_object($query)) {
      $this->enabled_modules[$row->name]['info'] = unserialize($row->info);
      $this->enabled_modules[$row->name]['weight'] = $row->weight; 
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
    $fingerprint = array();

    /** Populate depedencies **/
    /*
    foreach (module_list() as $module) {
      $export_options = $module .'_features_export_options';
      $data = array();
      if (function_exists($export_options)) {
        $data = $export_options();
      }
      $export['depedencies'][$module] = $data;
    }
    var_dump($export);

    foreach (module_list() as $module) {
      $export_options = $module .'_features_export_options';
      $data = array();
      if (function_exists($export_options)) {
        $data = $export_options();
      }

      $features_export = $module .'_features_export';
      if(function_exists($features_export)) {
        $xprt = $features_export($data, $export);
      }

//      $export = array();
      $export_render = $module .'_features_export_render';
      if (function_exists($export_render)) {
        $fingerprint[$module] = $export_render($module, $data, &$export);
      }
    }

    foreach (module_implements('features_api') as $module) {
      $info_hook = $module .'_features_api';
      $info = $info_hook();
      $default_hook = $info[$module]['default_hook'];
      var_dump($default_hook);
      $huk = module_invoke_all($default_hook);
      var_dump($huk);
    }

    $this->modules = $this->eval_fingerprint($fingerprint);
     */
    $stub = array();
    foreach (features_get_components(TRUE) as $component => $info) {
      if ($options = features_invoke($component, 'features_export_options')) {
        $stub[$component] = array_keys($options);
      }
    }
    $export = features_populate($stub, 'test');
    $all = features_export_render_hooks($export, 'test', TRUE);
    $all = $this->eval_fingerprint($all);
    var_dump($all);
    $files = features_export_render($export, 'test', TRUE);
   // var_dump($files);
    dd($files);
    
    /*
    return;
    $fingerprint = array();
    $module_name = 'test';
    $reset = FALSE;
    foreach (features_get_components(TRUE) as $module => $info) {
      $fingerprint[$module] = array();
      if ($options = features_invoke($module, 'features_export_options')) {
        foreach($options as $option_key => $text) {
          $stub[$module] = array($option_key);
          $export = features_populate($stub, 'test');

          $component_hooks = features_export_render_hooks($export, $module_name, $reset);

          // Group component code into their respective files
          foreach ($component_hooks as $component => $hooks) {
            foreach ($hooks as $hook_name => $hook_code) {
              //var_dump($hook_code);
      //        $fingerprint[$module][$option_key]['arr'] = @eval($hook_code);
              $fingerprint[$module][$option_key]['code'] = $hook_code;
            }
          }

        }
        //$fingerprint[$module]['__FULL__'] = features_export_render($export, 'test', TRUE);
        dd(features_export_render($export, 'test', TRUE));
      }
    }*/

    var_dump($fingerprint);
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
  public function get_modules($module_name) {
    if (isset($this->modules[$module_name])) {
      return $this->modules[$module_name];
    }

    return FALSE;
  }


  /**
   * Outputs the settings as string
   */
  public function get_output() {
    //return var_export(array('modules' => $this->modules, 'variables' => $this->variables), TRUE);
    //var_dump($this->enabled_modules);
    dd($this->modules);
    return '';
  }

}

