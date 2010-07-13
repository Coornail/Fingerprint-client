<?php

class ReverseDiff extends Diff {

  /**
   * @override
   */
  function Diff($from_lines, $to_lines) {
    $eng = new _ReverseDiffEngine();
    $this->edits = $eng->diff($from_lines, $to_lines);
    var_dump($this->edits);
  }
}


/**
 * @see _DiffEngine
 *
 * An engine that shows the differencies instead of the differencies
 */
class _ReverseDiffEngine extends _DiffEngine {

  /**
   * @override
   * Shows the similarities
   */
  function diff($from_lines, $to_lines) {
    $origdiff = new _DiffEngine();
    return $origdiff->diff($from_lines, $to_lines);
  }
}
