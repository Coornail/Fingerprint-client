<?php
// $Id$

/**
 * @file
 *   Fingerprint page callbacks
 */

/**
 * Sends the node to the Pronovix server
 * Callback for node/%node/fingerprint_send
 */
function fingerprint_send($node) {
  $url = variable_get('fingerprint_server', FINGERPRINT_DEFAULT_SERVER);
  $url .= variable_get('fingerprint_services_path', FINGERPRINT_DEFAULT_SERVICES_PATH);

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
  $remote_nid = new stdClass();
  $remote_nid->nid = $nid;
  $remote_nid->remote_nid = $remote_node;

  // If remote_nid is not set, we insert, else we update
  if ($node->fingerprint_remote_nid == FALSE) {
    $update = array();
  }
  else {
    $update = array('nid');
  }
  drupal_write_record('fingerprint', $remote_nid, $update);

  $remote_node_path = variable_get('fingerprint_server', FINGERPRINT_DEFAULT_SERVER) .'?q=node/'. $remote_node;
  return t('Node saved as !url', array('!url' => l('node/'. $remote_node, $remote_node_path)));
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
    '#default_value' => variable_get('fingerprint_server', FINGERPRINT_DEFAULT_DOMAIN),
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


/**
 * Lists nodes that the current node can be diffed for
 *
 * Callback for node/%/fingerprint_diff
 */
function fingerprint_diff_list($node) {
  global $user;
  $other_nodes = array();

  $result = db_query(db_rewrite_sql('SELECT nid, title FROM {node} WHERE type="fingerprint" AND nid<>%d AND uid=%d'), $node->nid, $user->uid);
  while ($other_node= db_fetch_object($result)) {
    $other_nodes[] = l($other_node->title, 'node/'. $node->nid .'/fingerprint_diff/'. $other_node->nid);
  }

  return theme('item_list', $other_nodes);
}


/**
 * Shows the difference between two fingerprint node
 *
 * Callback for node/%/fingerprint_diff/%
 *
 * @param $current int Current node id
 * @param $other int Other node id
 * @todo handle drupal_set_title in menu hook
 * @todo remove recursive references (since the hash will always be a difference)
 */
function fingerprint_view_diff($current, $other) {
  $current = node_load($current);
  $other = node_load($other);

  if (!(node_access('view', $current) || node_access('view', $other))) {
    drupal_access_denied();
  }

  drupal_set_title(t('Difference between "@current_title" and "@other_title"', array('@current_title' => $current->title, '@other_title' => $other->title)));

  // Emulate node render
  fingerprint_nodeapi(&$current, 'view', NULL, NULL, FINGERPRINT_NOHIGHLIGHT);
  fingerprint_nodeapi(&$other, 'view', NULL, NULL, FINGERPRINT_NOHIGHLIGHT);
  // Decoding '<' and '>' chars so it can be decoded and removing <br />'s
  $current->body = str_replace('<br />', '', htmlspecialchars_decode($current->content['body']['#value']));
  $other->body = str_replace('<br />', '', htmlspecialchars_decode($other->content['body']['#value']));

  module_load_include('inc', 'diff', 'diff.pages');
  $rows = _diff_body_rows($other, $current);
  $cols = _diff_default_cols();
  $header = _diff_default_header();

  return theme('diff_table', $header, $rows, array('class' => 'diff'), NULL, $cols);
}

