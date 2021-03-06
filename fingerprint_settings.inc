<?php
// $Id$

/**
 * @package fingerprint
 * @file
 *  Site fingerprint storage class
 */

// If the storage format changes, make sure to increase!
define('FINGERPRINT_VERSION', '0.2');

/**
 * Retrieves and stores the site settings
 */
class FingerprintSettings {
  /**
   * @var modules settings
   */
  protected $modules = array();
  public function getModules() {return $this->modules;}
  public function setModules($modules) {$this->modules = $modules;}

  /**
   * @var Variable settings
   */
  protected $variables = array();
  public function getVariables() {return $this->variables;}
  public function setVariables($variables) {$this->variables = $variables;}

  /**
   * @var Enabled modules
   */
  protected $enabledModules = array();
  public function getEnabledModules() {return $this->enabledModules;}
  public function setEnabledModules($enabledModules) {$this->enabledModules = $enabledModules;}

  /**
   * @var timestamp
   */
  protected $timestamp = array();
  public function getTimestamp() {return $this->timestamp;}
  public function setTimestamp($timestamp) {$this->timestamp = $timestamp;}

  /**
   * @var Status report
   */
  protected $statusReport = array();
  public function getStatusReport() {return $this->statusReport;}
  public function setStatusReport($report) {$this->statusReport = $report;}

  /**
   * @var version
   */
  protected $version = FINGERPRINT_VERSION;


  /**
   * When constructed it pulls in all the modules
   *
   * @todo change it to ondemand later
   */
  function  __construct($settings = array()) {
    if (!empty($settings)) {
      $this->importSettings($settings);
    }
    else {
      $this->pullVariables();
      $this->pullModules();
      $this->pullEnabledModules();
      $this->pullStatusReport();
      $this->setTimestamp(time());
    }
  }


  /**
   * Pull Status report
   */
  protected function pullStatusReport() {
    $report = array();
    foreach (module_implements('requirements') as $module) {
      $function = $module .'_requirements';
      $report[$module]['install'] = $function('install');
      $report[$module]['runtime'] = $function('runtime');
    }
    $this->setStatusReport($report);
  }


  /**
   * Imports settings that's exported from get_output_items()
   */
  protected function importSettings($settings) {
    if (!isset($settings['export_version']) || !$this->isImportSupported($settings['export_version'])) {
      throw new Exception("Importing version ". check_plain($settings['export_version']) .' doesn\'t match current version: '. FINGERPRINT_VERSION , E_USER_WARNING);
    }
    if (isset($settings['timestamp'])) {
      $this->setTimestamp($settings['timestamp']);
    }

    if (isset($settings['variables'])) {
      $this->setVariables($settings['variables']);
    }

    if (isset($settings['modules'])) {
      $this->setModules($settings['modules']);
    }

    if (isset($settings['enabled_modules'])) {
      $this->setEnabledModules($settings['enabled_modules']);
    }
    if (isset($settings['status_report'])) {
      $this->setStatusReport($settings['status_report']);
    }
  }


  /**
   * Pulls version info about the enabled modules
   */
  protected function pullEnabledModules() {
    $query = db_query('SELECT name, weight, status, info FROM {system}');
    while ($row = db_fetch_object($query)) {
      $this->enabledModules[$row->name]['info'] = unserialize($row->info);
      $this->enabledModules[$row->name]['weight'] = $row->weight;
      $this->enabledModules[$row->name]['status'] = (bool)$row->status;
    }
  }


  /**
   * Get all the variables into an internal storage
   */
  protected function pullVariables() {
    global $conf;
    $this->setVariables($conf);
  }


  /**
   * Get all the modules settings into the internal storage
   */
  protected function pullModules() {
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
    $this->setModules($this->evalFingerprint(array_merge_recursive($rendered_hooks, $fingerprint)));
  }


  /**
   * Evals the elements of the fingerprint to render it as an array
   */
  public static function evalFingerprint(Array &$fingerprint) {
    foreach ($fingerprint as $key => &$element) {
      if (is_string($element)) {
        $fingerprint[$key] = @eval($element);
      }
      elseif (is_array($element)) {
        FingerprintSettings::evalFingerprint($element);
      }
    }
    return $fingerprint;
  }


  /**
   * Outputs the settings as a serialized string
   */
  public function getOutput() {
    $ret = $this->getOutputItems();
    return serialize($ret);
  }


  /**
   * Returns the module info in an array
   * This has to be consistent across versions
   *
   * @param module name
   * @return Mixed Array of the module info, or FALSE if not found
   */
  public function moduleInfo(String $module) {
    if (!isset($this->moduleInfo[$module])) {
      return FALSE;
    }

    return array_merge($this->moduleInfo[$module], $this->modules[$module]);
  }


  /**
   * Returns the items that has to be in the output
   *
   * @return Array Associative array of the items and the names
   */
  protected function getOutputItems() {
    $ret['export_version'] = $this->version;
    $ret['timestamp'] = $this->getTimestamp();
    $ret['variables'] = $this->getVariables();
    $ret['modules'] = $this->getModules();
    $ret['enabled_modules'] = $this->getEnabledModules();
    $ret['status_report'] = $this->getStatusReport();

    return $ret;
  }


  /**
   * Converts an object to an array recursively
   * Resolves references by hashing the objects (for that you need the spl
   * extension)
   *
   * @todo handle recursive arrays
   *
   * @param Class a class object
   */
  protected static function objectToArrayRecursive(&$obj) {
    static $processed_objects = array();

    $object_hash = spl_object_hash($obj);
    if (in_array($object_hash, $processed_objects)) {
      $obj = '[reference] '. $object_hash;
      return;
    }
    $processed_objects[] = $object_hash;

    $obj_class = get_class($obj);

    /**
     * Workaround __PHP_Incomplete_Class
     * When unserialize() called and the class is not in the memory, php
     * doesn't know about the class.
     * Fortunately it saves the class name, so we can still build the array
     */
    if ($obj_class == '__PHP_Incomplete_Class') {
      $obj = (array)$obj;
      $obj_class = $obj['__PHP_Incomplete_Class_Name'];
      unset($obj['__PHP_Incomplete_Class']);
      unset($obj['__PHP_Incomplete_Class_Name']);
    }

    // We create the frist key with the classname, so no information is lost
    $arr[$obj_class] = (array)$obj;
    foreach ($arr[$obj_class] as $key => &$item) {
      if (is_object($item)) {
        FingerprintSettings::objectToArrayRecursive($item);
      }
    }
    $obj = $arr;
    $obj['hash'] = $object_hash;
  }


  /**
   * Checks if the version you want to import is supported
   */
  protected function isImportSupported($version) {
    if ($version == FINGERPRINT_VERSION)
      return TRUE;

    if ($version == '0.1' && FINGERPRINT_VERSION == '0.2' )
      return TRUE;

    return FALSE;
  }

}

