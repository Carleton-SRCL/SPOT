function [x_detrended, y_detrended] = remove_drift(x, y)
    % High-pass filter to remove drift
    hpFilt = designfilt('highpassiir', 'FilterOrder', 8, ...
                        'HalfPowerFrequency', 0.01, 'SampleRate', 1);
    x_detrended = filtfilt(hpFilt, x);
    y_detrended = filtfilt(hpFilt, y);
end
