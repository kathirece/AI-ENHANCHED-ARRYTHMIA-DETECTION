# Dataset format

`bpm_dataset_template.csv` contains only the expected sample-level header:

- `record`: source record identifier
- `time_s`: heartbeat time in seconds
- `rr_interval_s`: time between consecutive beats
- `bpm`: beats per minute
- `rate_class`: derived heart-rate category

Generated training data and preprocessing code are not included in this repository. Keep source record identifiers when evaluating a model so data from the same record is not accidentally used in both training and testing.
