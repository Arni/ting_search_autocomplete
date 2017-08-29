<?php
/**
 * @file
 * AJAX functions bibliotek.dk autocomplete.
 */

/**
 * Auto-complete callback for the ting search block.
 *
 * The function returns JSON and ending execution.
 *
 * @param string $string
 *   The string to search suggestion with.
 * @param bool $reset
 *   If TRUE the cache is reset. Defaults to FALSE.
 */
function ting_search_autocomplete_callback($query = '', $reset = FALSE) {
  $query = check_plain(strtolower($query));
  $long_query = false;
  $orig_query = '';
  if (strlen($query) > 3) {
    $orig_query = $query;
    $query = substr($query, 0, 3);
    $long_query = true;
  }

  $results = array();
  $suggestions = variable_get('ting_search_autocomplete_suggest_data');
  if(array_key_exists($query, $suggestions )) {
    $results = $suggestions[$query];
  }
  $matches = array();

  foreach ($results as $match) {
    if ($long_query) {
      if (!(substr($match, 0, strlen($orig_query)) === $orig_query)) { //If the query doesn't match skip this suggestion.
        continue;
      }
    }
    $key = utf8_encode ($match);
    $matches[truncate_utf8(strip_tags($key), 256, TRUE, FALSE, 1)] = utf8_encode ($match);
  }

  drupal_json_output($matches);
  drupal_exit();
}


/**
 * SuggestionService - retrieve suggestions with drupal_http_request() 
 *
 * @param string $query
 * @return array
 */
function ting_search_autocomplete_get_suggestions($query) {
  $items = array();
  $rest_url = variable_get('ting_search_autocomplete_suggestion_url', '');
  $method = variable_get('ting_search_autocomplete_method', 'facets');
  $settings = ting_search_autocomplete_settings();

  // minimumString is not an openSuggestion parameter
  unset($settings['minimumString']);

  $rest_url = $rest_url . 'rest/' . $method;
  $settings['query'] = $query;
  $url = url($rest_url, array('query' => $settings));

  $result = drupal_http_request($url, array());

  if (!empty($result->data)) {
    $result_data = drupal_json_decode($result->data);
    if (empty($result->error)) {
      if (!empty($result_data['suggestions'])) {
        foreach ($result_data['suggestions'] as $suggestion) {
          $items[truncate_utf8(strip_tags($suggestion['suggestion']), 256, TRUE, FALSE, 1)] = $suggestion['suggestion'];
        }
      }
    }
    else {
      watchdog('ting_search_autocomplete', 'Autocomplete error: ' . $result->error . ' (' . $result->code . ")\nURL: $url" , array(), $severity = WATCHDOG_ERROR);
    }
  }


  return $items;
}
