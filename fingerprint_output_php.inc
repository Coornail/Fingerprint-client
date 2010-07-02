<?php
require_once 'dna_output_interface.inc';

/**
 * Decorator class around dna_settings
 */
class php_export implements dna_output {
  /**
   * @var Internal storage of the settings
   */
  private $settings;

  function __construct(dna_settings $dna_settings) {
    $this->settings = $settings;

  }

  public function get_output() {
    return $dna_settings->get_output();
  }

}
