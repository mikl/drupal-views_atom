<?php

/**
* @file
* Views area text handler.
*/

/**
 * @ingroup views_area_handlers Views' area handlers
*/

class views_atom_handler_area_atom_tombstone extends views_handler_area {

  function option_definition() {
    $options = parent::option_definition();

        // Don't need content and format
    //unset($options['format'], $options['content']);

    $options['feed_url'] = array('default' => NULL);
    $options['hub_url'] = array('default' => NULL);
    $options['item_url'] = array('default' => NULL);
    $options['content'] = array('default' => '', 'translatable' => TRUE, 'format_key' => 'format');
    $options['format'] = array('default' => NULL);

    $options['use_push'] = array('default' => FALSE);

    // @todo, options could be added for updated, and email and name.

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['use_push'] = array(
      '#type' => 'checkbox',
      // @todo better title?
      '#title' => t('Use Push Hub'),
      '#default_value' => $this->options['use_push'],
    );

    $form['hub_url'] = array(
      '#title' => 'Hub URL',
      '#type' => 'textfield',
      '#default_value' => $this->options['hub_url'],
      // @todo, better description.
      '#description' => t('Enter an absolute path of a pubsubhubbub URL. You can also enter a path relative to this site which will then be transformed into an absolute.'),
      '#dependency' => array(
        'edit-options-use-push' => array(1),
      ),
    );

    $form['feed_url'] = array(
      '#title' => 'Feed URL',
      '#type' => 'textfield',
      '#default_value' => $this->options['feed_url'],
      // @todo, better description.
      '#description' => t('Enter an absolute path of a feed URL. You can also enter a path relative to this site which will then be transformed into an absolute.'),
    );

    // This will not work if the node was aliased when it existed.
    $form['item_url'] = array(
      '#title' => 'Item URL',
      '#type' => 'textfield',
      '#default_value' => $this->options['item_url'],
      // @todo, better description.
      '#description' => t('Enter an absolute path of the item to delete. You can also enter a path relative to this site which will then be transformed into an absolute.'),
    );

    $form['tokenize'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use replacement tokens from the first row'),
      '#default_value' => $this->options['tokenize'],
    );

    // Get a list of the available fields and arguments for token replacement.
    $options = array();
    foreach ($this->view->display_handler->get_handlers('field') as $field => $handler) {
      $options[t('Fields')]["[$field]"] = $handler->ui_name();
    }

    $count = 0; // This lets us prepare the key as we want it printed.
    foreach ($this->view->display_handler->get_handlers('argument') as $arg => $handler) {
      $options[t('Arguments')]['%' . ++$count] = t('@argument title', array('@argument' => $handler->ui_name()));
      $options[t('Arguments')]['!' . $count] = t('@argument input', array('@argument' => $handler->ui_name()));
    }

    if (!empty($options)) {
      $output = '<p>' . t('The following tokens are available. If you would like to have the characters %5B and %5D please use the html entity codes \'%5B\' or  \'%5D\' or they will get replaced with empty space.)' . '</p>');
      foreach (array_keys($options) as $type) {
        if (!empty($options[$type])) {
          $items = array();
          foreach ($options[$type] as $key => $value) {
            $items[] = $key . ' == ' . $value;
          }
          $output .= theme('item_list',
            array(
              'items' => $items,
              'type' => $type
            ));
        }
      }

      $form['token_help'] = array(
        '#type' => 'fieldset',
        '#title' => t('Replacement patterns'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $output,
        '#id' => 'edit-options-token-help',
        '#dependency' => array(
          'edit-options-tokenize' => array(1),
        ),
        '#prefix' => '<div>',
        '#suffix' => '</div>',
      );
    }
  }

  function render($empty = FALSE) {
    $format = isset($this->options['format']) ? $this->options['format'] : filter_default_format();
    if (!$empty || !empty($this->options['empty'])) {

      $updated = date('c', REQUEST_TIME);
      $tombstone_array =
      array(
        // @todo, what should title be, a field?
        'title' => t('Atom Tombstone'),
        'use_push' => $this->options['use_push'],
        'hub_url' => $this->render_option_as_url('hub_url'),
        'feed_url' => $this->render_option_as_url('feed_url'),
        // @todo Should this be when the node was last updated?
        'updated' => $updated,
        'items' => array(),
      );

      if (!empty($this->options['item_url'])) {
        // @todo, security on tokenize? What happens if something maliscious is used in "!1" ?
        $tombstone_array['items'][] = array(
          'guid' => $this->render_option_as_url('item_url'),
          'when' => $updated,
        );
      }

      return $this->render_textarea($this->options['content'], $format) .
       "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" . theme('views_atom_tombstone', $tombstone_array);
    }
    return '';
  }

  /**
   * Render a text area, using the proper format.
   */
  function render_textarea($value, $format) {
    if ($value) {
      if ($this->options['tokenize']) {
        $value = $this->view->style_plugin->tokenize_value($value, 0);
      }
      return check_markup($value, $format, '', FALSE);
    }
  }

  /**
   * Render a text area, using the proper format.
   */
  function render_option_as_url($option_name) {
    if (!empty($this->options[$option_name])) {
      if ($this->options['tokenize']) {
        $value = $this->view->style_plugin->tokenize_value($this->options[$option_name], 0);
      }
      else {
        $value = $this->options[$option_name];
      }
      return url($value, array('absolute' => TRUE));
    }
    else {
      return '';
    }
  }
}
