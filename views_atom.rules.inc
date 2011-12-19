<?php

/**
 * Implements hook_rules_action_info().
 */
function views_atom_rules_action_info() {
  $actions['views_atom_rules_action_add_to_feed'] = array(
    'label' => t('Add an entity to a feed'),
    'arguments' => array(
      'node' => array(
        'type' => 'node',
        'label' => t('Node'),
      ),
    ),
    'module' => 'Views Atom',
  );

  $actions['views_atom_rules_action_add_nids_to_feed'] = array(
    'label' => t('Add multiple nids to a feed'),
    'arguments' => array(
      'nids' => array(
        'type' => 'icalinstance_nids',
        'label' => t('Nids'),
      ),
    ),
    'module' => 'Views Atom',
  );

  return $actions;
}

/* ===== Action: Add To Feed ===== */

/**
 * Form callback for the views_atom_rules_action_add_to_feed action.
 */
function views_atom_rules_action_add_to_feed_form($settings, &$form) {

  foreach (views_atom_get_feed_displays() as $entry) {
    $options[$entry['title']][$entry['name'] . '-' . $entry['display']] = $entry['display_title'];
  }

  $form['generating_display'] = array(
    '#type' => 'select',
    '#title' => t('Generating view'),
    '#options' => $options,
    '#default_value' => $settings['view'] . '-' . $settings['view_display'],
    '#description' => t('Select the view and display that is responsible for generating the feed for this node. Only Feed displays are available.'),
  );
}

/**
 * Form submission callback for the views_atom_rules_action_add_to_feed action.
 */
function views_atom_rules_action_add_to_feed_submit(&$settings, $form, $form_state) {
  list($settings['view'], $settings['view_display']) = explode('-', $form_state['values']['generating_display']);
}

/**
 * Rule implementation callback for adding a node to a feed.
 *
 * @param $node
 *   The node that triggered this action.
 * @param $settings
 *   Configuration specified by the user.
 */
function views_atom_rules_action_add_to_feed($node, $settings) {
  $nid = $node->nid;

  $view = views_get_view($settings['view']);
  $view->set_display($settings['view_display']);

  if ($view->access($settings['view_display'])) {

    // If this node existed in the system prior to this page request, there is a chance
    // that it needs to be reloaded and thus node_load() needs to be reset.
    node_load($param = array(), $revision = NULL, $reset = TRUE);

    // Get the atom-formatted-data for this node.
    $result = $view->preview($settings['view_display'], array($nid));
    // Only PuSH the feed if there is actual data.  The view may have filtered
    // our nids down to 0, in which case we don't want to send anything. Because
    // normal View requests (Pull-based) are unaffected, Atom subscriptions still
    // work as normal.
    if (!empty($view->result)) {
      $topic_url = url($view->display[$view->current_display]->display_options['path'], array('absolute' => TRUE));
      // This uses the Drupal Queue module to actually send notifications later.
      push_hub_notify($topic_url, $result);
    }
  }
}

/* ===== Action: Add Nids To Feed ===== */

/**
 * Form callback for the viws_atom_rules_action_add_to_feed action.
 */
function views_atom_rules_action_add_nids_to_feed_form($settings, &$form) {

  foreach (views_atom_get_feed_displays() as $entry) {
    $options[$entry['title']][$entry['name'] . '-' . $entry['display']] = $entry['display_title'];
  }

  $form['generating_display'] = array(
    '#type' => 'select',
    '#title' => t('Generating view'),
    '#options' => $options,
    '#default_value' => $settings['view'] . '-' . $settings['view_display'],
    '#description' => t('Select the view and display that is responsible for generating the feed for these nodes. Only Feed displays are available.'),
  );

  $form['settings']['max_push_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum push size'),
    '#default_value' => $settings['max_push_size'],
    '#description' => t('This action may have a very large number of nodes passed to it. It may be beneficial to break this list of nodes into smaller chunks before passing the chunks on to Push Hub. Enter a positive integer as the maximum number of nodes to be sent to Push Hub together. If you enter 40 in this field and this action is called on a list of 120 nodes there will be three chunks of 40 nodes sent to Push Hub. Leave this field blank or enter 0 to always send the list of nodes to Push Hub as one chunk, no matter how large.'),
    '#element_validate' => array('views_atom_element_validate_integer_positive_or_zero'),
  );
}

/**
 * Helper form element validator: integer >= 0.
 *
 * This is a backport, and slight modification, of Drupal 7's _element_validate_integer_positive().
 * This function allows zero.
 */
function views_atom_element_validate_integer_positive_or_zero($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value || $value < 0)) {
    form_error($element, t('%name must be a positive integer or 0.', array('%name' => $element['#title'])));
  }
}

/**
 * Form submission callback for the views_atom_rules_action_add_to_feed action.
 */
function views_atom_rules_action_add_nids_to_feed_submit(&$settings, $form, $form_state) {
  list($settings['view'], $settings['view_display']) = explode('-', $form_state['values']['generating_display']);
}

/**
 * Rule implementation callback for adding multiple nodes to a feed.
 *
 * @param $nids
 *   An array of nids that we want to push using the configured view.
 * @param $settings
 *   Configuration specified by the user.
 */
function views_atom_rules_action_add_nids_to_feed($nids, $settings) {

  if ($nids) {
    // The $nids array will be broken into smaller chunks.
    if (is_numeric($settings['max_push_size']) && $settings['max_push_size'] >= 1) {
      $chunk_size = $settings['max_push_size'];
    }
    // If the 'max_push_size' setting is left blank or is 0, use the size of the
    // $nids array as the chunk size.
    else {
      $chunk_size = count($nids);
    }

    // Break the array of nids into smaller pieces so that
    // push_hub_notify() doesn't get overly large packages.
    $chunked_array = array_chunk($nids, $chunk_size);

    // Loop through each chunk to render it as atom and send it with push_hub.
    foreach ($chunked_array as $nids_chunk) {
      $view = views_get_view($settings['view']);
      $view->set_display($settings['view_display']);

      if ($view->access($settings['view_display'])) {
        $nid_string = implode(',', $nids_chunk);

        // If these nodes existed in the system prior to this page request, there is a chance
        // that they need to be reloaded and thus node_load() needs to be reset.
        node_load($param = array(), $revision = NULL, $reset = TRUE);

        // Get the atom-formatted-data for these nodes.
        $result = $view->preview($settings['view_display'], array($nid_string));
        // Only PuSH the feed if there is actual data.  The view may have filtered
        // our nids down to 0, in which case we don't want to send anything. Because
        // normal View requests (Pull-based) are unaffected, Atom subscriptions still
        // work as normal.
        if (!empty($view->result)) {
          $topic_url = url($view->display[$view->current_display]->display_options['path'], array('absolute' => TRUE));
          // This uses the Drupal Queue module to actually send notifications later.
          push_hub_notify($topic_url, $result);
        }
      }
    }
  }
}

