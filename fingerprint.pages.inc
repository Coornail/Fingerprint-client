<?php

/**
 * Sends the node to the Pronovix server
 * Callback for node/%node/fingerprint_send
 */
function fingerprint_send($node) {
  $url = variable_get('fingerprint_server', FINGERPRINT_DEFAULT_SERVER);
  $domain = variable_get('fingerprint_domain', '');
  $key = variable_get('fingerprint_key', '');
  $username = $domain;
  $password = variable_get('fingerprint_password', '');

  if (empty($url) || empty($domain) || empty($key) || empty($password)) {
    return t('First you have to set up your credentials: !url', array('!url' => l('Fingerprint settings', 'admin/settings/fingerprint')));
  }

  module_load_include('inc', 'fingerprint', 'fingerprint_xmlrpc');
  $xmlrpc_handler = new FingerprintXmlrpc($domain, $key, $url);
  if (!$xmlrpc_handler->session_active()) {
    return t('Can\'t connect to server. Please !check_url.', array('!check_url' => l('check the server url', 'admin/settings/fingerprint')));
  }

  $user = $xmlrpc_handler->userLogin($username, $password);
  if (!$user) {
    return t('Authentication failed: %message', array('%message' => xmlrpc_error_msg()));
  }

  // we'll need it later
  $nid = $node->nid;

  unset($node->fingerprint);
  /**
   * If we heaven't saved the node yet, we'll create a new one
   * For that we have to remove the nid, else we'll just update
   */
  if ($node->fingerprint_remote_nid === FALSE) {
    unset($node->nid);
  }
  // Have to set changed, else the version on the other server will be never, and node can't be saved
  $node->changed = time();


  $node->uid = $user->uid;
  $node->status = FALSE;
  $remote_node = $xmlrpc_handler->send('node.save', array($node));
  if (!$remote_node) {
    return t('Failed to save node: %message', array('%message' => xmlrpc_error_msg()));
  }

  // Write remote nid to the database
  $remote_nid = new StdClass();
  $remote_nid->nid = $nid;
  $remote_nid->remote_nid = $remote_node;

  // If remote_nid is not set, we insert, else we update
  if ($node->fingerprint_remote_nid == FALSE) {
    $update = array();
  } else {
    $update = array('nid');
  }
  drupal_write_record('fingerprint', $remote_nid, $update);

  // If we own the domain we can provide nice url
  if (strpos($url, FINGERPRINT_DEFAULT_DOMAIN) !== FALSE) {
    $remote_node_path = FINGERPRINT_DEFAULT_DOMAIN .'?q=remote_node/'. $remote_node;
    return t('Node saved as !url', array('!url' => l('remote_node/'. $remote_node, $remote_node_path)));
  }

  return t('Node saved as node/%nid', array('%nid' => $remote_node));
}


/**
 * System settings form of the module
 *
 * Callback for admin/settings/fingerprint
 */
function fingerprint_settings_form($form_state) {
  $form = array();

  $form['fingerprint_server'] = array(
    '#type' => 'textfield',
    '#title' => t('Server'),
    '#default_value' => variable_get('fingerprint_server', FINGERPRINT_DEFAULT_SERVER),
    '#description' => t('If you want to use your own fingerprint store, the following services permissions should be enabled: %permissions', array('%permissions' => 'system.connect, user.login, node.save')),
    '#required' => TRUE,
  );

  $form['fingerprint_domain'] = array( // We're using the username as the domain as well
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => variable_get('fingerprint_domain', ''),
    '#required' => TRUE,
  );

  $form['fingerprint_password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#default_value' => variable_get('fingerprint_password', ''),
    '#required' => TRUE,
  );

  $form['fingerprint_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Api key'),
    '#default_value' => variable_get('fingerprint_key', ''),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

