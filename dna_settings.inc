<?php

/**
 * @file Site dna storage class
 */

require_once 'dna_output_interface.inc';

/**
 *
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

  }

  /**
   * Get all the modules settings into an internal storage
   */
  private function pull_modules() {

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
