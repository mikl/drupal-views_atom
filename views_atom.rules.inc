<?php

/**
 * Implements hook_rules_action_info().
 */
function views_atom_rules_action_info() {
  $actions['views_atom_rules_action_add_to_feed'] = array(
    'label' => t('Add an entity to a feed'),
    'group' => t('Views Atom'),
    'arguments' => array(
      'node' => array(
        'type' => 'node',
        'label' => t('Node'),
      ),
    ),
    'parameter' => array(
      // @todo, should this be "node" instead of "entity" ?
      'data' => array(
        'type' => 'entity',
        'label' => t('Entity'),
        'description' => t(''),
        'restriction' => 'selector',
        'wrapped' => TRUE,
      ),
      'view_display' => array(
        'type' => 'list<text>',
        'label' => t('View and Display'),
        'options list' => 'views_atom_get_feed_displays_options',
        'description' => t('Select the view and display that is responsible for generating the feed for this node. Only Feed displays are available.'),
      ),
    ),
    'base' => 'views_atom_rules_action_add_to_feed',
  );

  return $actions;
}

/**
 * Returns views_atom displays in a form usable for Rules forms.
 *
 * @return array
 */
function views_atom_get_feed_displays_options() {

  $options = array();
  foreach (views_atom_get_feed_displays() as $entry) {
    $options[$entry['title']][$entry['name'] . '-' . $entry['display']] = $entry['display_title'];
  }

  return $options;
}

/**
 * Rule implementation callback for adding a node to a feed.
 *
 * @param $node
 *   The node that triggered this action.
 * @param $settings
 *   Configuration specified by the user.
 */
function views_atom_rules_action_add_to_feed($data, $views_display, $settings, RulesState $state, RulesPlugin $element) {

  // @todo, ideal this rules_action would go in push_hub or a more generic atom module.
  if (module_exists('push_hub')) {

    // @todo, revisit this variable name.
    $views_displays = $views_display;

    // @todo, question whether Views is necessary at all for the atomization.
    // parts of render() inside of views_plugin_row_rdf_node could be abstracted out.
    $nid = $data->getIdentifier();

    foreach($views_displays as $view_name_and_display) {
      list($view_name, $view_display_name) = explode('-', $view_name_and_display);

      $view = views_get_view($view_name);
      $view->set_display($view_display_name);

      if ($view->access($view_display_name)) {

        // If this node existed in the system prior to this page request, there is a chance
        // that it needs to be reloaded and thus node_load() needs to be reset.
        // @todo, revisit this for D7.
        node_load(NULL, NULL, $reset = TRUE);

        // Get the atom-formatted-data for this node.
        $result = $view->preview($view_display_name, array($nid));


        // Only PuSH the feed if there is actual data.  The view may have filtered
        // our nids down to 0, in which case we don't want to send anything. Because
        // normal View requests (Pull-based) are unaffected, Atom subscriptions still
        // work as normal.
        if (!empty($view->result)) {
          $topic_url = url($view->get_url(), array('absolute' => TRUE));

          // This uses the Drupal Queue module to actually send notifications later.
          // @todo, need a check that push_hub exists. Maybe in action_info.
          push_hub_notify($topic_url, $result);
        }
      }
    }
  }
}

