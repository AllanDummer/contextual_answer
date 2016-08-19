<?php

/**
 * Contextual answer classes.
 *
 * @file
 *  Classes modelling the long answer question and the contextual answer question response
 */

/**
 * Extension of QuizQuestion.
 */
class ContextualAnswerQuestion extends QuizQuestion {
  
  // Constants for answers options
  const ANSWER_MATCH = 0;
  const ANSWER_INSENSITIVE_MATCH = 1;

  /**
   * Implementation of saveNodeProperties
   *
   * @see QuizQuestion#saveNodeProperties($is_new)
   */
  public function saveNodeProperties($is_new = FALSE) {
    if (! isset($this->node->feedback)) {
      $this->node->feedback = '';
    }
    
    if (is_array($this->node->correct_answers)) {
      $this->node->correct_answers = serialize($this->node->correct_answers);
    }
    
    $fields = array(
        'max_score' => (int) $this->node->max_score,
        'answer_score' => isset($this->node->answer_score) ? $this->node->answer_score : 0,
        'correct_text' => $this->node->correct_text,
        'correct_answers' => $this->node->correct_answers,
        'correct_answer_evaluation' => isset($this->node->correct_answer_evaluation) ? $this->node->correct_answer_evaluation : 0,
        'show_feedback' => isset($this->node->show_feedback) ? (int) $this->node->show_feedback : 0,
        'minimum_percentage' => isset($this->node->minimum_percentage) ? (int) $this->node->minimum_percentage : 0,
        'feedback_pass' => isset($this->node->feedback_pass['value']) ? $this->node->feedback_pass['value'] : '',
        'feedback_fail' => isset($this->node->feedback_fail['value']) ? $this->node->feedback_fail['value'] : '' 
    );
    
    if ($is_new || $this->node->revision == 1) {
      $fields['nid'] = $this->node->nid;
      $fields['vid'] = $this->node->vid;
      $id = db_insert('quiz_contextual_answer_node_properties')->fields($fields)
        ->execute( );
    }
    else {
      db_update('quiz_contextual_answer_node_properties')->fields($fields)
        ->condition('nid', $this->node->nid)
        ->execute( );
    }
  }

  /**
   * Implementation of validateNode
   *
   * @see QuizQuestion#validateNode($form)
   */
  public function validateNode(array &$form) {
    
    // ddl($this, 'validateNode');
    if ($this->node->correct_answer_evaluation && empty($this->node->correct_answers)) {
      form_set_error('correct_answers', t('At least one question, answer pair must be provided.'));
    }
    else {
      $this->node->correct_answers = trim($this->node->correct_answers);
      
      $this->node->correct_answers = str_replace('&', '&amp;', $this->node->correct_answers);
      $answers = explode("\n", $this->node->correct_answers);
      
      $body = str_replace(array(
          '<strong>',
          '</strong>' 
      ), array(
          '',
          '' 
      ), $this->node->body['und'][0]['value']);
      
      $pos = 0;
      
      foreach ($answers as $row) {
        
        $array = explode("|", $row);
        
        if (count($array) > 1) {
          $question = trim($array[0]);
          $answer = trim($array[1]);
          
          if (isset($array[2])) {
            $skip = trim($array[2]);
            $skip = (int) $skip;
            if ($skip > 0) {
              while ($skip != 0) {
                $pos = stripos($body, $question, $pos);
                $skip -= 1;
              }
            }
            else {
              $pos = stripos($body, $question, $pos);
            }
          }
          else {
            $pos = stripos($body, $question, $pos);
          }
          
          if ($pos === FALSE) {
            form_set_error('correct_answers', t('The question <strong>' . $question . '</strong> does not appear in the Contextual paragraph or it is out of sequence in the answers.<br />Questions must be an exact match to work correctly.'));
            break;
          }
          else {
            $pos = $pos + strlen($question);
          }
        }
        else {
          form_set_error('correct_answers', t('<strong>' . $row . '</strong> seems to be a miss matched question or it does not have an answer.<br />Question, Answers must be paired and seperated by a pipe character - | - and must be an exact match.'));
          break;
        }
      }
    }
    
    if ($this->node->answer_score <= 0) {
      form_set_error('answer_score', t('The answer score must be one or more.'));
    }
    if ($this->node->show_feedback) {
      if ($this->node->minimum_percentage < 1 || $this->node->minimum_percentage > 100) {
        form_set_error('minimum_percentage', t('The minimum percentage must be between 1 and a 100.'));
      }
      if (empty($this->node->feedback_pass['value'])) {
        form_set_error('feedback_pass', t('You need to give a pass feedback message.'));
      }
      if (empty($this->node->feedback_fail['value'])) {
        form_set_error('feedback_fail', t('You need to give a fail feedback message.'));
      }
    }
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestion#delete($only_this_version)
   */
  public function delete($only_this_version = FALSE) {
    $delete_node = db_delete('quiz_contextual_answer_node_properties');
    $delete_node->condition('nid', $this->node->nid);
    
    if ($only_this_version) {
      $delete_node->condition('vid', $this->node->vid);
    }
    
    $delete_node->execute( );
    parent::delete($only_this_version);
  }

  /**
   * Implementation of getNodeProperties
   *
   * @see QuizQuestion#getNodeProperties()
   */
  public function getNodeProperties( ) {
    if (isset($this->nodeProperties) && ! empty($this->nodeProperties)) {
      return $this->nodeProperties;
    }
    
    if (isset($this->node->nid)) {
      $props = array();
      $props['max_score'] = isset($this->node->max_score) ? trim($this->node->max_score) : '';
      $props['feedback']['value'] = isset($this->node->show_feedback) ? trim($this->node->show_feedback) : '';
      $props['feedback']['format'] = 'basic';
      $props['is_quiz_question'] = TRUE;
      
      $res_a = db_query('SELECT correct_text, correct_answers, max_score FROM {quiz_contextual_answer_node_properties}
        WHERE nid = :nid AND vid = :vid', array(
          ':nid' => $this->node->nid,
          ':vid' => $this->node->vid 
      ))->fetchAssoc( );
      
      $correct_answers = unserialize($res_a['correct_answers']);
      $res_a['correct_answers'] = str_replace('&', '&amp;', $correct_answers);
      
      if (is_array($res_a)) {
        $props = array_merge($props, $res_a);
      }
      
      $this->nodeProperties = $props;
      return $props;
    }
  }

  /**
   * Implementation of getNodeView
   *
   * @see QuizQuestion#getNodeView()
   */
  public function getNodeView( ) {
    $content = parent::getNodeView( );
    
    // $this->getNodeProperties();
    
    if ($this->viewCanRevealCorrect( )) {
      
      $body = strip_tags($this->node->body['und'][0]['safe_value']);
      
      $question = '';
      
      $correct_answers = '<thead><tr><th class="contextual-answer">Question</th>' . '<th class="contextual-correct-answer">Correct Answer</th></tr></thead><tbody>';
      
      $rows = $this->node->correct_answers;
      
      $wrong = FALSE;
      $pos = 0;
      
      foreach ($rows as $row) {
        
        $question = $row['question'];
        $pos = stripos($body, $question, $pos);
        
        if ($pos === FALSE) {
          $question_classes = 'contextual-answer wrong';
          $wrong = TRUE;
        }
        else {
          $question_classes = 'contextual-answer';
          $pos = $pos + strlen($question);
        }
        
        $correct_answers .= '<tr><td class="' . $question_classes . '">' . $row['question'] . '</td><td class="' . $question_classes . '">' . $row['correct_answer'] . '</td></tr>';
        $question = $row['question'];
      }
      
      /*
       * $content['correct_text_body'] = array(
       * '#title' => t('Correct answers'),
       * '#label_display' => 'above',
       * '#markup' => '<div class="quiz-solution field-body">' . $this->node->correct_text . '</div>',
       * '#weight' => 10,
       * );
       *
       */
      
      $content['correct_answers'] = array(
          '#title' => t('Correct answers.'),
          '#label_display' => 'above',
          '#markup' => '<div class="quiz-solution"><table>' . trim($correct_answers) . '</tbody></table></div>',
          '#weight' => 12 
      );
      
      if ($wrong) {
        drupal_set_message(t('The are answers that do not appear in the contextual paragraph. Please correct them.'), 'error');
      }
    }
    else {
      
      $content['correct_answers'] = array(
          '#title' => t('Correct answers.'),
          '#label_display' => 'above',
          '#markup' => serialize($this->node->correct_answers), // Pass the correct answers to the question form or entity if needed.
          '#access' => FALSE,
          '#weight' => 2 
      );
      
      $content['correct_text'] = array(
          '#title' => t('Correct text.'),
          '#label_display' => 'above',
          '#markup' => $this->node->correct_text, // Pass the correct answers to the question form or entity if needed.
          '#access' => FALSE,
          '#weight' => 3 
      );
    }
    
    return $content;
  }

  /**
   * Implementation of getAnweringForm
   *
   * @see QuizQuestion#getAnsweringForm($form_state, $result_id)
   */
  public function getAnsweringForm(array $form_state = NULL, $result_id) {
    $element = parent::getAnsweringForm($form_state, $result_id);
    
    $element += array(
        '#type' => 'textfield',
        '#title' => t('Answer'),
        '#description' => t('Enter your answer here'),
        '#default_value' => '',
        '#size' => 60,
        '#maxlength' => 256,
        '#required' => TRUE,
        '#attributes' => array(
            'autocomplete' => 'off' 
        ) 
    );
    
    // if (isset($result_id)) {
    // $response = new ContextualAnswerResponse($result_id, $this->node);
    // $element['#default_value'] = $response->getResponse();
    // }
    return $element;
  }

  public function getAnsweringFormValidate(array &$element, &$value) {
    $this->node->answers[0]['answer'] = $value['answer'];
    $this->node->answers[0]['is_evaluated'] = $value['is_evaluated'];
    $this->node->answers[0]['score'] = $value['score'];
  }

  /**
   * Implementation of getCreationForm
   *
   * @see QuizQuestion#getCreationForm($form_state)
   */
  public function getCreationForm(array &$form_state = NULL) {
    $this->getNodeProperties( );
    $form['answers'] = array(
        '#type' => 'fieldset',
        '#title' => t('Correct answers'),
        '#description' => t('Provide the answers and the method by which the answer will be evaluated.'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#weight' => 10 
    );
    
    $options = array(
        self::ANSWER_MATCH => t('Automatic and case sensitive'),
        self::ANSWER_INSENSITIVE_MATCH => t('Automatic. Not case sensitive') 
    );
    
    $feedback_options = array(
        0 => 'No',
        1 => 'Yes' 
    );
    
    $form['answers']['correct_answer_evaluation'] = array(
        '#type' => 'radios',
        '#title' => t('Pick an evaluation method'),
        '#description' => t('Choose how the answer shall be evaluated.'),
        '#options' => $options,
        '#default_value' => isset($this->node->correct_answer_evaluation) ? $this->node->correct_answer_evaluation : self::ANSWER_MATCH,
        '#required' => FALSE 
    );
    
    if (isset($this->node->correct_answers) && ! empty($this->node->correct_answers)) {
      $rows = $this->node->correct_answers;
      $correct_answers = '';
      if (is_array($rows)) {
        
        foreach ($rows as $row) {
          
          $question = $row['question'];
          $answer = $row['correct_answer'];
          $skip = 0;
          
          if (isset($row['skip'])) {
            $skip = $row['skip'];
          }
          
          $correct_answers .= $question . ' | ' . $answer . ' | ' . $skip . "\n";
        }
        $this->node->correct_answers = str_replace('&amp;', '&', $correct_answers);
      }
    }
    else {
      $this->node->correct_answers = '';
    }
    
    $form['answers']['correct_answers'] = array(
        '#type' => 'textarea',
        '#title' => t('Correct answers'),
        '#description' => t('Specify the answers. The format must be "exact question match"|"correct answer" | "skip number of same words (optional)",<br /> one per line, and must be in the order that they are in the contextual.'),
        '#default_value' => isset($this->node->correct_answers) ? trim($this->node->correct_answers) : '',
        '#size' => 60,
        '#rows' => 15,
        '#required' => TRUE 
    );
    
    $form['answers']['answer_score'] = array(
        '#type' => 'textfield',
        '#title' => t('Score per answer'),
        '#description' => t('The score a Learner will get for each question answered correctly. Must be greater than zero.'),
        '#size' => 3,
        '#default_value' => isset($this->node->answer_score) ? $this->node->answer_score : variable_get('contextual_question_default_answer_score', 1),
        '#required' => TRUE 
    );
    
    $form['answers']['correct_text'] = array(
        '#type' => 'textarea',
        '#access' => FALSE 
    );
    $form['answers']['max_score'] = array(
        '#type' => 'textfield',
        '#access' => FALSE 
    );
    $form['feedback'] = array(
        '#type' => 'fieldset',
        '#title' => t('Feedback'),
        '#description' => t('Provide feedback to show when all the questions has been answered.'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#weight' => 10 
    );
    $form['feedback']['show_feedback'] = array(
        '#type' => 'radios',
        '#title' => t('Show feedback'),
        '#options' => $feedback_options,
        '#default_value' => isset($this->node->show_feedback) ? $this->node->show_feedback : 0,
        '#required' => FALSE 
    );
    $form['feedback']['container'] = array(
        '#type' => 'container',
        '#states' => array(
            'visible' => array(
                ':input[name="show_feedback"]' => array(
                    'value' => 1 
                ) 
            ) 
        ) 
    );
    
    $form['feedback']['container']['minimum_percentage'] = array(
        '#type' => 'textfield',
        '#title' => 'Minimum pass percentage',
        '#description' => 'A score above this percentage will be shown the Feedback Pass text. Below this percentage will be shown the Feedback fail text.',
        '#size' => 3,
        '#default_value' => isset($this->node->minimum_percentage) ? $this->node->minimum_percentage : NULL,
        '#required' => FALSE 
    );
    
    $form['feedback']['container']['feedback_pass'] = array(
        '#title' => t('Pass feedback'),
        '#type' => 'text_format',
        '#description' => t('The text entered here would be shown to learners who pass this question.'),
        '#default_value' => isset($this->node->feedback_pass) ? $this->node->feedback_pass : '',
        '#format' => 'basic' 
    );
    
    $form['feedback']['container']['feedback_fail'] = array(
        '#title' => t('Fail feedback'),
        '#type' => 'text_format',
        '#description' => t('The text entered here would be shown to learners who fail this question.'),
        '#default_value' => isset($this->node->feedback_fail) ? $this->node->feedback_fail : '',
        '#format' => 'basic' 
    );
    return $form;
  }

  /**
   * Implementation of getMaximumScore
   *
   * @see QuizQuestion#getMaximumScore()
   */
  public function getMaximumScore( ) {
    if (! isset($this->nodeProperties['max_score'])) {
      $this->getNodeProperties( );
    }
    return $this->nodeProperties['max_score'];
  }
}

/**
 * Extension of QuizQuestionResponse
 */
class ContextualAnswerResponse extends QuizQuestionResponse {

  /**
   * Get all scores that have not yet been evaluated.
   *
   * @param $count Number
   *          of items to return (default: 50).
   * @param $offset Where
   *          in the results we should start (default: 0).
   *          
   * @return Array of objects describing unanswered questions. Each object will have result_id, question_nid, and question_vid.
   */
  public static function fetchAllUnscoredAnswers($count = 50, $offset = 0) {
    global $user;
    $query = db_select('quiz_contextual_answer_user_answers', 'a');
    $query->fields('a', array(
        'answer_feedback',
        'answer_feedback_format' 
    ));
    $query->fields('r', array(
        'title' 
    ));
    $query->fields('qnr', array(
        'time_end',
        'time_start',
        'uid' 
    ));
    $query->fields('qnra', array(
        'result_id',
        'question_nid',
        'question_vid' 
    ));
    $query->join('quiz_node_results_answers', 'qnra', 'a.result_answer_id = qnra.result_answer_id');
    $query->join('node_revision', 'r', 'qnra.question_vid = r.vid');
    $query->join('quiz_node_results', 'qnr', 'qnra.result_id = qnr.result_id');
    $query->join('node', 'n', 'qnr.nid = n.nid');
    $query->condition('a.is_evaluated', 0);
    if (user_access('score own quiz') && user_access('score taken quiz answer')) {
      $query->condition(db_or( )->condition('n.uid', $user->uid)
        ->condition('qnr.uid', $user->uid));
    }
    elseif (user_access('score own quiz')) {
      $query->condition('n.uid', $user->uid);
    }
    elseif (user_access('score taken quiz answer')) {
      $query->condition('qnr.uid', $user->uid);
    }
    $results = $query->execute( );
    $unscored = array();
    foreach ($results as $row) {
      $unscored[] = $row;
    }
    return $unscored;
  }

  /**
   * Given a quiz, return a list of all of the unscored answers.
   *
   * @param $nid Node
   *          ID for the quiz to check.
   * @param $vid Version
   *          ID for the quiz to check.
   * @param $count Number
   *          of items to return (default: 50).
   * @param $offset Where
   *          in the results we should start (default: 0).
   *          
   * @return Indexed array of result IDs that need to be scored.
   */
  public static function fetchUnscoredAnswersByQuestion($nid, $vid, $count = 50, $offset = 0) {
    $results = db_query('SELECT result_id FROM {quiz_contextual_answer_user_answers}
      WHERE is_evaluated = :is_evaluated
      AND question_nid = :question_nid
      AND question_vid = :question_vid', array(
        ':is_evaluated' => 0,
        ':question_nid' => $nid,
        ':question_vid' => $vid 
    ));
    $unscored = array();
    foreach ($results as $row) {
      $unscored[] = $row->result_id;
    }
    return $unscored;
  }
  
  /**
   * ID of the answer.
   */
  protected $answer_id = 0;
  private $answer_feedback;
  private $answer_feedback_format;

  /**
   * Constructor
   */
  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);
    
    if (! isset($answer)) {
      // Question has been answered already. We fetch the answer data from the database.
      $r = db_query('SELECT *
        FROM {quiz_contextual_answer_user_answers}
        WHERE result_answer_id = :raid', array(
          ':raid' => $this->result_answer_id 
      ))->fetch( );
      
      if (! empty($r)) {
        $this->answer = $r->answer;
        $this->score = $r->score;
        $this->evaluated = $r->is_evaluated;
        $this->answer_id = $r->answer_id;
        $this->answer_feedback = $r->answer_feedback;
        $this->answer_feedback_format = $r->answer_feedback_format;
      }
    }
    else {
      $this->answer = $answer;
      $this->evaluated = FALSE;
    }
  }

  /**
   * Implementation of save
   *
   * @see QuizQuestionResponse#save()
   */
  public function save( ) {
    
    // We sent the answer with it's score in as an array. Move everything back
    // so the rest of the classes will know where it is. A bit ugly but it works.
    $this->question->answers[0]['is_evaluated'] = $this->question->answers[0]['answer']['is_evaluated'];
    unset($this->question->answers[0]['answer']['is_evaluated']);
    $score = $this->question->answers[0]['answer']['score'];
    $this->question->answers[0]['score'] = $score;
    unset($this->question->answers[0]['answer']['score']);
    $answer = $this->question->answers[0]['answer']['answer'];
    $this->question->answers[0]['answer'] = $answer;
    
    if ($score > 0) {
      db_merge('quiz_contextual_answer_user_answers')->key(array(
          'result_answer_id' => $this->result_answer_id 
      ))
        ->fields(array(
          'result_answer_id' => $this->result_answer_id,
          'score' => $this->question->answers[0]['score'],
          'is_evaluated' => $this->question->answers[0]['is_evaluated'],
          'answer' => $this->question->answers[0]['answer'],
          'answer_feedback' => $this->quizQuestion->nodeProperties['feedback']['value'],
          'answer_feedback_format' => $this->quizQuestion->nodeProperties['feedback']['format'] 
      ))
        ->execute( );
    }
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestionResponse#delete()
   */
  public function delete( ) {
    db_delete('quiz_contextual_answer_user_answers')->condition('result_answer_id', $this->result_answer_id)
      ->execute( );
  }

  public function score( ) {
    if (isset($this->question->answers[0]['score']) && $this->question->answers[0]['score'] > 0) {
      return $this->question->answers[0]['score'];
    }
  }

  /**
   * Implementation of getResponse
   *
   * @see QuizQuestionResponse#getResponse()
   */
  public function getResponse( ) {
  }

  /**
   * Returns a renderable array of question feedback.
   */
  public function getFeedback( ) {
    $out = array();
    
    $node = node_load($this->question->nid, $this->question->vid);
    $entity_info = entity_get_info('node');
    
    foreach ($entity_info['view modes'] as $view_mode => $info) {
      if ($this->canReview("quiz_question_view_" . $view_mode)) {
        node_build_content($node, $view_mode);
        unset($node->content['answers']);
        unset($node->content['links']);
        
        $type = 'node';
        drupal_alter(array(
            'node_view',
            'entity_view' 
        ), $node->content, $type);
        
        $out['question'][$node->nid] = $node->content;
      }
    }
    
    $rows = array();
    
    $labels = array(
        'attempt' => t('Your answer'),
        'solution' => t('Correct answer') 
    );
    
    drupal_alter('quiz_feedback_labels', $labels);
    $feedback = $this->getFeedbackValues( );
    
    foreach ($feedback as $idx => $row) {
      foreach ($labels as $reviewType => $label) {
        if ((isset($row[$reviewType]) && ! empty($row[$reviewType]))) {
          $rows[$idx][$reviewType] = $row[$reviewType];
        }
      }
    }
    
    if (arg(4) == 'feedback') {
      $class = 'quiz-score';
      $out['score_display']['#markup'] = theme('contextual_answer_quiz_question_score', array(
          'score' => $this->question->answers[0]['score'],
          'max_score' => $this->question->max_score,
          'class' => $class 
      ));
    }
    
    if ($rows) {
      $headers = array_intersect_key($labels, $rows[0]);
      $type = $this->getQuizQuestion( )->node->type;
      $markup = theme('quiz_question_feedback__' . $type, array(
          'labels' => $headers,
          'data' => $rows 
      ));
      
      // Add the answer feedback below the table.
      if (! empty($feedback['answer_feedback'])) {
        $markup .= '<div class="answer-feedback">' . $feedback['answer_feedback'] . '</div>';
      }
      $out['response']['#markup'] = $markup;
    }
    
    if ($this->canReview('question_feedback')) {
      if ($properties = entity_load('quiz_question', FALSE, array(
          'nid' => $this->quizQuestion->node->nid,
          'vid' => $this->quizQuestion->node->vid 
      ))) {
        $quiz_question = reset($properties);
        $quiz_result = quiz_result_load($this->result_id);
        $account = user_load($quiz_result->uid);
        $token_types = array(
            'global' => NULL,
            'node' => $this->quizQuestion->node,
            'user' => $account 
        );
        $quiz_question->feedback = token_replace($quiz_question->feedback, $token_types);
        $out['question_feedback']['#markup'] = check_markup($quiz_question->feedback, $quiz_question->feedback_format);
      }
    }
    
    if ($this->canReview('score')) {
      $out['max_score'] = array(
          '#type' => 'value',
          '#value' => $this->getMaxScore( ) 
      );
      
      if ($this->isEvaluated( )) {
        
        $score = $this->getScore( );
        $max_score = $this->getMaxScore( );
        
        if (($score / $max_score) >= .5) {
          $class = 'q-correct';
        }
        else {
          $class = 'q-wrong';
        }
      }
      else {
        $score = t('?');
        $class = 'q-waiting';
      }
      
      $out['score_display']['#markup'] = theme('quiz_question_score', array(
          'score' => $score,
          'max_score' => $this->getMaxScore( ),
          'class' => $class 
      ));
      $out['score'] = array(
          '#type' => 'value',
          '#value' => $this->score( ) 
      );
    }
    
    // /ddl ( $this, 'This feedback' );
    return $out;
  }

  /**
   * Implementation of getReportFormResponse
   */
  public function getFeedbackValues( ) {
    if (! isset($this->quizQuestion->nodeProperties)) {
      $this->quizQuestion->nodeProperties( );
    }
    
    $data = array();
    
    $data[] = array(
        // Hide these column. Does not make sense for contextual question as there are no choices.
        'choice' => NULL,
        'correct' => NULL,
        'score' => ! $this->question->answers[0]['score'] ? t('This answer has not yet been scored.') : $this->getScore( ),
        'attempt' => $this->question->answers[0]['answer'],
        'solution' => $this->quizQuestion->nodeProperties['correct_text'],
        'answer_feedback' => check_markup($this->answer_feedback, $this->answer_feedback_format) 
    );
    
    return $data;
  }

  public function getReportFormAnswerFeedback( ) {
    return array(
        '#title' => t('Enter feedback'),
        '#type' => 'text_format',
        '#default_value' => isset($this->answer_feedback) ? check_markup($this->answer_feedback, $this->answer_feedback_format) : '',
        '#format' => isset($this->answer_feedback_format) ? $this->answer_feedback_format : filter_default_format( ),
        '#weight' => 20,
        '#attributes' => array(
            'class' => array(
                'quiz-report-score' 
            ) 
        ) 
    );
  }

  /**
   * Implementation of getReportFormSubmit
   *
   * @see QuizQuestionResponse#getReportFormSubmit()
   */
  public function getReportFormSubmit( ) {
    return 'contextual_answer_report_submit';
  }

  /**
   * Implements QuizQuestionResponse::getReportFormValidate().
   */
  public function getReportFormValidate(&$element, &$form_state) {
    $max = $this->question->max_score;
    // Check to make sure that entered score is not higher than max allowed score.
    if ($element['score']['#value'] > $max) {
      form_error($element['score'], t('The score needs to be a number between @min and @max', array(
          '@min' => 0,
          '@max' => $max 
      )));
    }
  }
}
