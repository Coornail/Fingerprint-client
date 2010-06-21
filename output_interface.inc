<?php

/**
 * Interface for output plugins for the dna export
 */
interface dna_output {
  /**
   * @var Internal storage of the settings
   */
  private $settings;

  public __construct($settings);

  /**
   * Output provider
   *
   * @returns String The desired output
   */
  public get_output();
}
