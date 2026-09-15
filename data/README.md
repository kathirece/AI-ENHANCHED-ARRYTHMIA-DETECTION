# Dataset outputs

Run `notebooks/mit_bih_rr_to_bpm.ipynb` in Google Colab to create:

- `mit_bih_bpm_samples.csv`: one row per valid RR interval, with record ID, time, RR interval, BPM, and the derived rate category.
- `mit_bih_bpm_windows.csv`: record-contained windows with `bpm_0` through `bpm_9` plus one label.

Generated data is ignored by Git to avoid accidentally committing a large derived dataset. Keep the record ID during evaluation so samples from one MIT-BIH record do not appear in both training and test sets.

`bpm_dataset_template.csv` contains only the expected sample-level header.

