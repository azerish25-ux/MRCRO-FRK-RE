package com.mfrk.renderercomparison;

import android.app.Activity;
import android.os.Bundle;
import android.os.Build;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

/** Test-only activity: no launcher shortcut, audio engine, or instrument façade. */
public final class RendererComparisonActivity extends Activity {
    private RendererComparisonView comparison;
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        comparison = new RendererComparisonView(this);
        comparison.assignNormalized(getIntent().getFloatExtra("cutoff",1),
            getIntent().getFloatExtra("resonance",0),getIntent().getFloatExtra("envelope",0.5f));
        setContentView(comparison); hideBars();
    }
    @Override public void onWindowFocusChanged(boolean focused) {
        super.onWindowFocusChanged(focused);if (focused) hideBars();
    }
    private void hideBars() {
        if (Build.VERSION.SDK_INT>=30) {
            getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController insets=getWindow().getInsetsController();
            if (insets!=null) {
                insets.hide(WindowInsets.Type.systemBars());
                insets.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN|View.SYSTEM_UI_FLAG_HIDE_NAVIGATION|
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY|View.SYSTEM_UI_FLAG_LAYOUT_STABLE|
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN|View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    }
    public RendererComparisonView getComparisonView() { return comparison; }
}
