package io.github.isaacrecomp;

import android.Manifest;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.content.Context;
import android.util.DisplayMetrics;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

import java.io.File;

/**
 * SDL activity of IsaacRecomp. The native program (libmain.so) calls the static methods below through JNI:
 * where the data folder is, whether we may read it, and how to restart after "Delete save".
 */
public class IsaacActivity extends SDLActivity {
    private static IsaacActivity sSelf;
    private static volatile boolean sWantStorage = false;
    private AlertDialog mDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        sSelf = this;
        super.onCreate(savedInstanceState);
    }

    /**
     * The game may render at a lower resolution than the screen (SurfaceHolder.setFixedSize: the display hardware
     * scales it up for free). SDL normalizes touches by the surface's buffer size; with a fixed size the touch
     * coordinates are still in screen pixels, so normalize by the view's size instead.
     */
    static class IsaacSurface extends SDLSurface {
        IsaacSurface(Context context) { super(context); }
        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            super.surfaceChanged(holder, format, width, height);
            if (getWidth() > 0 && getHeight() > 0) { mWidth = getWidth(); mHeight = getHeight(); }
        }
    }

    @Override
    protected SDLSurface createSDLSurface(Context context) {
        return new IsaacSurface(context);
    }

    /** Shorter side of the screen in pixels (landscape: its height). */
    public static int screenHeight() {
        if (sSelf == null) return 0;
        DisplayMetrics m = new DisplayMetrics();
        sSelf.getWindowManager().getDefaultDisplay().getRealMetrics(m);
        return Math.min(m.widthPixels, m.heightPixels);
    }

    /** Render at this height (keeping the screen's aspect ratio); 0 or the screen height = native resolution. */
    public static void setRenderHeight(final int h) {
        if (sSelf == null) return;
        sSelf.runOnUiThread(() -> {
            if (mSurface == null) return;
            DisplayMetrics m = new DisplayMetrics();
            sSelf.getWindowManager().getDefaultDisplay().getRealMetrics(m);
            int W = Math.max(m.widthPixels, m.heightPixels), H = Math.min(m.widthPixels, m.heightPixels);
            // the view must keep filling the screen: SDL adds it without layout params, and a SurfaceView measures
            // itself from the fixed buffer size (it showed in a corner at 1:1)
            mSurface.setLayoutParams(new android.widget.RelativeLayout.LayoutParams(
                android.view.ViewGroup.LayoutParams.MATCH_PARENT, android.view.ViewGroup.LayoutParams.MATCH_PARENT));
            if (h <= 0 || h >= H) mSurface.getHolder().setSizeFromLayout();
            else mSurface.getHolder().setFixedSize(Math.round((float) W * h / H) & ~1, h);
        });
    }

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }

    @Override
    protected String[] getArguments() {
        String[] a = getIntent() != null ? getIntent().getStringArrayExtra("args") : null;
        return a != null ? a : new String[0];
    }

    /** Full screen: no status or navigation bar (a swipe from the edge shows them for a moment), notch area used. */
    @SuppressWarnings("deprecation")
    private void goImmersive() {
        if (Build.VERSION.SDK_INT >= 28) {
            WindowManager.LayoutParams lp = getWindow().getAttributes();
            lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
            getWindow().setAttributes(lp);
        }
        if (Build.VERSION.SDK_INT >= 30) {
            getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController c = getWindow().getInsetsController();
            if (c != null) {
                c.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
                c.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) goImmersive();
    }

    @Override
    protected void onResume() {
        super.onResume();
        goImmersive();
        // back from the system settings: ask again if access is still missing
        if (sWantStorage) {
            if (hasStorageAccess()) { if (mDialog != null) mDialog.dismiss(); mDialog = null; }
            else showStorageDialog();
        }
    }

    /**
     * The data folder (the game, isaac.ini, saves, mods, log.txt), the first that exists of:
     * 1. the folder written in IsaacRecomp/datadir.txt of the internal storage (its first line, a full path), so it
     *    can be anywhere, e.g. /storage/1234-5678/Games/Isaac;
     * 2. an IsaacRecomp folder at the root of another storage (SD card, USB drive), e.g. /storage/1234-5678/IsaacRecomp;
     * 3. the IsaacRecomp folder at the root of the internal storage (/storage/emulated/0/IsaacRecomp), created if needed.
     * (issue #46: devices with little internal storage). The native program logs which one it uses (log.txt there).
     */
    public static String rootDir() {
        File internal = new File(Environment.getExternalStorageDirectory(), "IsaacRecomp");
        String custom = firstLine(new File(internal, "datadir.txt"));
        if (custom != null) {
            File d = new File(custom);
            d.mkdirs();
            if (d.isDirectory() && d.canWrite()) return d.getAbsolutePath();
        }
        if (sSelf != null) {
            File[] dirs = sSelf.getExternalFilesDirs(null);   // (one per storage: <volume>/Android/data/<app>/files)
            if (dirs != null) for (File f : dirs) {
                if (f == null) continue;
                String p = f.getAbsolutePath();
                int i = p.indexOf("/Android/");
                if (i <= 0) continue;
                File d = new File(p.substring(0, i), "IsaacRecomp");
                if (!d.getAbsolutePath().equals(internal.getAbsolutePath()) && d.isDirectory() && d.canWrite()) return d.getAbsolutePath();
            }
        }
        internal.mkdirs();
        return internal.getAbsolutePath();
    }

    private static String firstLine(File f) {
        if (!f.isFile()) return null;
        try (java.io.BufferedReader r = new java.io.BufferedReader(new java.io.InputStreamReader(new java.io.FileInputStream(f), "UTF-8"))) {
            String l;
            while ((l = r.readLine()) != null) {
                l = l.replace("\uFEFF", "").trim();
                if (!l.isEmpty() && !l.startsWith(";") && !l.startsWith("#")) return l;
            }
        } catch (java.io.IOException e) { /* (unreadable: the default folders) */ }
        return null;
    }

    public static boolean hasStorageAccess() {
        if (Build.VERSION.SDK_INT >= 30) return Environment.isExternalStorageManager();
        return sSelf == null || sSelf.checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }

    /** Called by the native program at startup when access is missing; it then waits until hasStorageAccess(). */
    public static void requestStorageAccess() {
        sWantStorage = true;
        if (sSelf != null) sSelf.runOnUiThread(() -> sSelf.showStorageDialog());
    }

    private void showStorageDialog() {
        if (mDialog != null && mDialog.isShowing()) return;
        String msg = Build.VERSION.SDK_INT >= 30
            ? "Isaac Recomp reads the game (Isaac.exe) and keeps its settings and saves in the \"IsaacRecomp\" folder of your internal storage (or of your SD card).\n\nAllow \"All files access\" on the next screen, then come back."
            : "Isaac Recomp reads the game (Isaac.exe) and keeps its settings and saves in the \"IsaacRecomp\" folder of your internal storage (or of your SD card).\n\nAllow access to your files on the next screen.";
        mDialog = new AlertDialog.Builder(this)
            .setTitle("Storage access")
            .setMessage(msg)
            .setCancelable(false)
            .setPositiveButton("Continue", (d, w) -> {
                mDialog = null;
                if (Build.VERSION.SDK_INT >= 30) {
                    try {
                        startActivity(new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION, Uri.parse("package:" + getPackageName())));
                    } catch (Exception e) {
                        startActivity(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION));
                    }
                } else {
                    requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE }, 1);
                }
            })
            .setNegativeButton("Quit", (d, w) -> { mDialog = null; finishAndRemoveTask(); System.exit(0); })
            .create();
        mDialog.show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 1 && !hasStorageAccess()) showStorageDialog();
    }

    /**
     * "Delete save": start again with the same game selected. The native side exits right after this; a small
     * activity in another process (RestartActivity) waits for that and launches the game again.
     */
    public static void restartApp(String game) {
        if (sSelf == null) return;
        Intent i = new Intent(sSelf, RestartActivity.class);
        i.putExtra("args", new String[] { "--game", game });
        i.putExtra("pid", android.os.Process.myPid());
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        sSelf.startActivity(i);
    }
}
