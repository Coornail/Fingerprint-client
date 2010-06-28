<?php
require_once 'dna_output_interface.inc';

/**
 * Decorator class around dna_settings
 */
class dna_xml extends dna_settings {


  function __construct() {
    parent::__construct();
  }


  /**
   * @override
   *
   * Outputs the module info as xml
   */
  public function get_output() {
    $ret = $this->get_output_items();
    $xml = $this->toXML($ret);

    // Only domdocument can indent xml, convert
    $dom = dom_import_simplexml($xml)->ownerDocument;
    $dom->formatOutput = TRUE;
    return $dom->saveXML(); 
  }


  /**
   * Parses php array into xml
   *
   * mostly copied from http://snipplr.com/view.php?codeview&id=3491
   */
  private function toXML(array $data, $rootNodeName = 'data', $xml=null) {
    // turn off compatibility mode as simple xml throws a wobbly if you don't.
    if (ini_get('zend.ze1_compatibility_mode') == 1) {
      ini_set ('zend.ze1_compatibility_mode', 0);
    }

    if ($xml == null) {
      $xml = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><'. $rootNodeName .' /> ');
    }

    foreach($data as $key => $value) {
       if (empty($value)) {
         continue;
       }

      // no numeric keys
      if (is_numeric($key)) {
        $key = 'element';
      }

      if (is_object($value)) {
        dna_xml::object_to_array_recursive($value);
      }

      // replace anything not alpha numeric or '_'
      $key = preg_replace('/[^a-z_]/i', '', $key);

      if (is_array($value)) {
        $node = $xml->addChild($key);
        // recrusive call.
        $this->toXml($value, $rootNodeName, $node);
      } else {
        // add single node.
        $value = htmlentities($value);
        $xml->addChild($key, $value);
      }

    }

    return $xml;
  }


  /**
   * Formats xml to be well structured, indented
   * and higlighted as html
   *
   * @return String html output
   */
  public static function format_xml($xml) {
    require_once drupal_get_path('module', 'geshifilter') .'/geshifilter.pages.inc';
    return geshifilter_process($xml, 'XML');
  }


  /**
   * Converts an object to an array recursively
   * Resolves references by hashing the objects (for that you need the spl
   * extension)
   *
   * @param Class a class object
   */
  private static function object_to_array_recursive(&$obj) {
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
        dna_xml::object_to_array_recursive($item);
      }
    }
    $obj = $arr;
    $obj['hash'] = $object_hash;
  }

}
