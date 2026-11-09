# Dataset format and scope

`bpm_dataset_template.csv` is a template for a **heart-rate category** dataset. Its columns are:

- `record`: source record identifier
- `time_s`: heartbeat time in seconds
- `rr_interval_s`: time between consecutive ECG R peaks, if the source is ECG
- `bpm`: derived heart-rate value
- `rate_class`: simplified rate category (for example, Bradycardia / Normal / Tachycardia)

This template does not contain the MIT-BIH dataset, generated training samples, preprocessing code, or verified labels. Keep source record and subject identifiers so that related samples do not leak across training and test sets.

**Do not describe BPM-threshold labels as MIT-BIH arrhythmia beat labels.** They represent heart-rate categories only. A genuine ECG arrhythmia classifier needs beat/rhythm annotations and waveform-compatible model inputs, not just BPM thresholds. See [the concept redesign](../docs/concept-redesign.md).
