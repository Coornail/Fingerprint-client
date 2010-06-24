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
    $ret = array();
    $ret['timestamp'] = $this->timestamp;
    $ret['variables'] = $this->variables;
    $ret['modules'] = $this->modules;
    var_dump($this->toXML($ret));
    return $this->toXML($ret);
  }


  /**
   * Parses php array into xml
   *
   * mostly copied from http://snipplr.com/view.php?codeview&id=3491
   *
   * @todo damn views object
   */
  private function toXML(array $data, $rootNodeName = 'data', $xml=null) {
    // turn off compatibility mode as simple xml throws a wobbly if you don't.
    if (ini_get('zend.ze1_compatibility_mode') == 1) {
      ini_set ('zend.ze1_compatibility_mode', 0);
    }

    if ($xml == null) {
      $xml = simplexml_load_string("<?xml version='1.0' encoding='utf-8'?><$rootNodeName />");
    }

    // loop through the data passed in.
    foreach($data as $key => $value) {
      // no numeric keys in our xml please!
      if (is_numeric($key)) {
        $key = "unknownNode_". (string) $key;
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
        $xml->addChild($key,$value);
      }

    }

    return $xml->asXML();
  }

}
