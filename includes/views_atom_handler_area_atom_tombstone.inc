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
    $options['hub_url'] = array('default' => NULL);
    $options['content'] = array('default' => '', 'translatable' => TRUE, 'format_key' => 'format');
    $options['format'] = array('default' => NULL);
    $options['tokenize'] = array('default' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['hub_url'] = array(
      '#title' => 'Hub URL',
      '#type' => 'textfield',
      '#default_value' => $this->options['hub_url'],
      // @todo, better description.
      '#description' => t('Enter an absolute path of a pubsubhubbub URL. You can also enter a path relative to this site which will then be transformed into an absolute.'),
    );

    $form['content'] = array(
      '#type' => 'text_format',
      '#default_value' => $this->options['content'],
      '#rows' => 6,
      '#format' => isset($this->options['format']) ? $this->options['format'] : filter_default_format(),
      '#wysiwyg' => FALSE,
      '#access' => FALSE,
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

  function options_submit(&$form, &$form_state) {
    $form_state['values']['options']['format'] = $form_state['values']['options']['content']['format'];
    $form_state['values']['options']['content'] = $form_state['values']['options']['content']['value'];
    parent::options_submit($form, $form_state);
  }

  function render($empty = FALSE) {
    $format = isset($this->options['format']) ? $this->options['format'] : filter_default_format();
    if (!$empty || !empty($this->options['empty'])) {


      $tombstone_array =
      array(
        'title' => 'Test Title',
        'link' => NULL,
        'use_push' => TRUE,

        // Need a field for this

        // @todo, security?
        'hub_url' => url($this->view->style_plugin->tokenize_value($this->options['hub_url'], 0), array('absolute' => TRUE)),

        // @todo Need a field for this
        'feed_url' => 'feed_url',  // http://d7upgrades.persch.sandbox/node/54/atom

        // Should this be when the node was last updated?
        'updated' => NULL,

         // @todo, how is this diffent from feed URL
        'link' => NULL,
        'items' => array(),
      );



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

}
