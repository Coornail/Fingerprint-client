<?php
/**
 *
 *Originally from drupal.org/node/774298
 */
class FingerprintXmlrpc {

  function __construct( $domain = '', $apiKey = '', $endPoint = '', $verbose = FALSE ) {
    // set local domain or IP address
    // this needs to match the domain set when you created the API key
    $this->domain = $domain;

    // set API key
    $this->kid = $apiKey;

    // set target web service endpoint
    $this->endpoint = $endPoint;

    // extended debugging
    $this->verbose = $verbose;

    // call system.connect to get our required anonymous sessionId:
    $retVal = $this->send( 'system.connect', array() );
    $this->session_id = $retVal['sessid'];

    if ($this->verbose) {
      $func = 'DrupalXmlrpc->__construct:';
      if ($this->session_id) {
        var_dump( $func.' got anonymous session id fine' );
      } 
      else{
        var_dump( $func.' failed to get anonymous session id!' );
      }
    }
  }


  /***********************************************************************
  * Function for sending xmlrpc requests
  */
  public function send( $methodName, $functionArgs = array() ) {
    $protocolArgs = array();
    // only the system.connect method does not require a sessionId:
    if ($methodName == 'system.connect') {
      $protocolArgs = array( $this->endpoint, $methodName );
    }
    else {
      $timestamp = (string)time();
      $nonce = $this->getUniqueCode("10");

      // prepare a hash
      $hash_parameters = array( $timestamp, $this->domain, $nonce, $methodName );
      $hash = hash_hmac("sha256", implode(';', $hash_parameters), $this->kid);

      // prepared the arguments for this service:
      // note, the sessid needs to be the one returned by user.login
      $protocolArgs = array( $this->endpoint, $methodName, $hash, $this->domain, $timestamp, $nonce, $this->session_id );
    }

    $params = array_merge( $protocolArgs, $functionArgs );
    return call_user_func_array('xmlrpc', $params);
  }

  /***************************************************
  * login and return user object
  */
  public function userLogin($userName = '', $userPass = '') {
    if ($this->verbose)
      var_dump( 'DrupalXmlrpc->userLogin() called with userName "'.$userName.'" and pass "'.$userPass.'"' );

    // clear out any lingering xmlrpc errors:
    xmlrpc_error( NULL, NULL, TRUE );

    $retVal = $this->send('user.login', array($userName, $userPass));
    if (!$retVal && xmlrpc_errno()) {
      if ($this->verbose)
        var_dump( 'userLogin() failed! errno "'.xmlrpc_errno().'" msg "'.xmlrpc_error_msg().'"' );
      return FALSE;
    }
    else {
      // remember our logged in session id:
      $this->session_id = $retVal['sessid'];

      // we might need the user object later, so save it:
      $user = new stdClass();
      $user = (object)$response['user'];
      $this->authenticated_user = $user;
      return $user;
    }
  }


  public function userLogout() {
    $retVal = $this->send( 'user.logout', array() );
    if (!$retVal) {
      if ($this->verbose)
      var_dump( 'userLogout() failed! errno "'.xmlrpc_errno().'" msg "'.xmlrpc_error_msg().'"' );
      return FALSE;
    }

    return TRUE;
  }

  /***************************************************
  * Function for generating a random string, used for
  * generating a token for the XML-RPC session
  */
  private function getUniqueCode($length = 10) {
    return user_password($length);
  }
}
