<div class="quiz-report-score-container <?php print $class ?>">
  <span>
    <?php
    $perc = round(($score / $max_score) * 100, 0);
    $percentage = ' - (' . $perc . '%)';
    ?>
    <?php print t('Score:') ?> <?php print $score ?> <?php print t('answers correct out of a possible') . ' ' . $max_score ?><?php print $percentage ?>
  </span>
</div>
