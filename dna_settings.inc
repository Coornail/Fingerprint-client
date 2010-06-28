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
  protected $modules = array();

  /**
   * @var variable settings
   */
  protected $variables = array();

  /**
   * @var enabled modules
   */
  protected $enabled_modules = array();

  /**
   * @var timestamp
   */
  protected $timestamp = array();

  /**
   * @var version
   */
  protected $version = DNA_VERSION;


  /**
   * When constructed it pulls in all the modules
   *
   * @todo change it to ondemand later
   */
  function  __construct($settings = array()) {
    if (!empty($settinsg)) {
      $this->import_settings($settings);
    } else {
      $this->pull_variables();
      $this->pull_modules();
      $this->pull_enabled_modules();
      $this->timestamp = time();
    }
  }


  /**
   * Imports settings that's exported from get_output_items()
   */
  protected function import_settings($settings) {
    if (!isset($ret['export_version']) || $ret['export_version'] == DNA_VERSION) {
      throw new Exception ("Importing version ". $ret['export_version'] .' doesn\'t match current version: '. DNA_VERSION , E_USER_WARNING);
    }
    if (isset($ret['timestamp'])) {
      $this->timestamp = $settings['timestamp'];
    }

    if (isset($ret['variables'])) {
      $this->timestamp = $settings['variables'];
    }

    if (isset($ret['modules'])) {
      $this->timestamp = $settings['modules'];
    }

    if (isset($ret['enabled_modules'])) {
      $this->timestamp = $settings['enabled_modules'];
    }
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
  protected function pull_modules() {
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
  public static function eval_fingerprint(Array &$fingerprint) {
    foreach ($fingerprint as $key => &$element) {
      if (is_string($element)) {
        $fingerprint[$key] = @eval($element);
      } else if (is_array($element)) {
        dna_settings::eval_fingerprint($element);
      }
    }
    return $fingerprint;
  }

  /**
   * Outputs the settings as a serialized string
   */
  public function get_output() {
    $ret = $this->get_output_items();
    return serialize($ret);
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

    return array_merge($this->module_info[$module], $this->modules[$module]);
  }


  /**
   * Returns the items that has to be in the output
   *
   * @return Array Associative array of the items and the names
   */
  protected function get_output_items() {
    $ret['export_version'] = $this->version;
    $ret['timestamp'] = $this->timestamp;
    $ret['variables'] = $this->variables;
    $ret['modules'] = $this->modules;
    $ret['enabled_modules'] = $this->enabled_modules;
    
    return $ret;
  }


  /**
   * Converts an object to an array recursively
   * Resolves references by hashing the objects (for that you need the spl
   * extension)
   *
   * @param Class a class object
   */
  protected static function object_to_array_recursive(&$obj) {
    static $processed_objects = array();

    $object_hash = spl_object_hash($obj);
    if (in_array($object_hash, $processed_objects)) {
      $obj = '[reference] '. $object_hash;
      return;
    }
    $processed_objects[] = $object_hash;

    $obj_class = get_class($obj);
    // We create the frist key with the classname, so no information is lost
    $arr[$obj_class] = (array)$obj;
    foreach($arr[$obj_class] as $key => &$item) {
      if (is_object($item)) {
        dna_settings::object_to_array_recursive($item);
      }
    }
    $obj = $arr;
    $obj['hash'] = $object_hash;
  }

}

