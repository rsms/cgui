#pragma once

void _gui_init_main(int argc, const char** argv, gui_t*);
void _gui_before_start(gui_t*);
void _gui_onFramebufferDidChange(gui_t*);

struct TimeMeasure {
  static const size_t NSAMPLES = 120;
  double      _start = 0.0;
  double      _avg = 0.0;
  double      _samples[NSAMPLES] = {0};
  uint32_t    _samplei = 0;
  double avg() {
    double sum_ms = 0.0;
    for (uint32_t i = 0; i < NSAMPLES; i++) {
      sum_ms += _samples[i] * 1000.0;
    }
    return (sum_ms / (double)NSAMPLES) / 1000.0;
  }
  void start() {
    _start = gui_monotime();
  }
  void end() {
    double d = gui_monotime() - _start;
    _samples[_samplei++ % NSAMPLES] = d;
  }
};
