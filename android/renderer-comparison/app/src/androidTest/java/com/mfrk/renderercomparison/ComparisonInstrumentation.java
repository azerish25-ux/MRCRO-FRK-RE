package com.mfrk.renderercomparison;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.RectF;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.ViewTreeObserver;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.HashSet;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import org.json.JSONArray;
import org.json.JSONObject;

/** Remote emulator smoke/metamorphic screenshots, not an original-pixel oracle. */
public final class ComparisonInstrumentation extends Instrumentation {
    @Override public void onCreate(Bundle arguments) { super.onCreate(arguments); start(); }
    @Override public void onStart() {
        Bundle result=new Bundle();
        try {
            File directory=new File(getTargetContext().getFilesDir(),"renderer-comparison");
            if (!directory.isDirectory() && !directory.mkdirs()) throw new AssertionError("Output directory");
            verifyAssets();
            String[] names={"minimum","center","factory-default","maximum","factory-repeat"};
            float[][] values={{0,0,0},{0.5f,0.5f,0.5f},{1,0,0.5f},{1,1,1},{1,0,0.5f}};
            JSONArray cases=new JSONArray(); HashSet<String> unique=new HashSet<>(); String factoryHash=null;
            Intent intent=new Intent(getTargetContext(),RendererComparisonActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            RendererComparisonActivity activity=(RendererComparisonActivity)startActivitySync(intent);
            RendererComparisonView view=activity.getComparisonView();
            for (int index=0;index<names.length;++index) {
                final float[] assigned=values[index];
                CountDownLatch frame=new CountDownLatch(1);
                runOnMainSync(() -> {
                    boolean[] queued={false};
                    ViewTreeObserver.OnDrawListener[] listener=new ViewTreeObserver.OnDrawListener[1];
                    listener[0]=() -> {
                        if (queued[0]) return;
                        queued[0]=true;
                        view.post(() -> view.getViewTreeObserver().removeOnDrawListener(listener[0]));
                        view.postOnAnimation(() -> view.postOnAnimation(frame::countDown));
                    };
                    view.getViewTreeObserver().addOnDrawListener(listener[0]);
                    view.assignNormalized(assigned[0],assigned[1],assigned[2]);
                });
                if (!frame.await(10,TimeUnit.SECONDS)) throw new AssertionError("Frame timeout");
                waitForIdleSync();SystemClock.sleep(100);
                RectF[] content=new RectF[1];int[] location=new int[2];JSONObject[] audit=new JSONObject[1];boolean[] focused=new boolean[1];
                runOnMainSync(() -> {
                    content[0]=view.contentPixels();view.getLocationOnScreen(location);audit[0]=view.audit();focused[0]=view.hasWindowFocus();
                });
                if (view.getWidth()==0 || view.drawnFrames()==0) throw new AssertionError("No native view frame");
                JSONArray assignments=audit[0].getJSONArray("assignments");
                for (int knob=0;knob<3;++knob)
                    if ((float)assignments.getJSONObject(knob).getDouble("normalized")!=values[index][knob])
                        throw new AssertionError("Parameter assignment changed");
                Bitmap screen=getUiAutomation().takeScreenshot();
                if (screen==null) throw new AssertionError("UiAutomation screenshot unavailable");
                RectF rect=content[0];int x=location[0]+(int)Math.floor(rect.left),y=location[1]+(int)Math.floor(rect.top);
                int width=(int)Math.ceil(rect.right)-(int)Math.floor(rect.left);
                int height=(int)Math.ceil(rect.bottom)-(int)Math.floor(rect.top);
                if (x<0 || y<0 || x+width>screen.getWidth() || y+height>screen.getHeight())
                    throw new AssertionError("Comparison crop outside screenshot");
                Bitmap crop=Bitmap.createBitmap(screen,x,y,width,height);
                String hash=pixelsHash(crop);if (index<4) unique.add(hash);
                if (index==2) factoryHash=hash;
                save(screen,new File(directory,names[index]+"-viewport.png"));
                save(crop,new File(directory,names[index]+"-crop.png"));
                audit[0].put("window_focused",focused[0]);
                audit[0].put("scenario",names[index]);audit[0].put("crop_argb_sha256",hash);
                audit[0].put("capture_pixels",new JSONArray(new int[]{x,y,width,height}));
                audit[0].put("pixel_oracle","none: original native raster is not verified");cases.put(audit[0]);
                JSONObject partial=new JSONObject();partial.put("passed",false);partial.put("cases",cases);
                try (FileOutputStream output=new FileOutputStream(new File(directory,"comparison-report.json"))) {
                    output.write(partial.toString(2).getBytes(StandardCharsets.UTF_8));
                }
                if (crop!=screen) crop.recycle();screen.recycle();
                if (!audit[0].getBoolean("window_focused")) throw new AssertionError("Comparison obscured by another window");
                if (index==4 && !hash.equals(factoryHash)) throw new AssertionError("Repeated assignment not deterministic");
            }
            runOnMainSync(activity::finish);waitForIdleSync();
            if (unique.size()!=4) throw new AssertionError("Indicator endpoint/center frames did not change");
            JSONObject report=new JSONObject();report.put("passed",true);report.put("cases",cases);
            report.put("assertions","required asset hashes, native draw, assignments, crop bounds, distinct positions, repeat stability");
            try (FileOutputStream output=new FileOutputStream(new File(directory,"comparison-report.json"))) {
                output.write(report.toString(2).getBytes(StandardCharsets.UTF_8));
            }
            result.putString("stream","Renderer comparison smoke checks passed; original pixel parity remains unverified.\n");
            finish(Activity.RESULT_OK,result);
        } catch (Throwable error) {
            android.util.Log.e("RendererComparison","Instrumentation failure",error);
            result.putString("stream","Renderer comparison failed: "+error+"\n");finish(Activity.RESULT_CANCELED,result);
        }
    }
    private void verifyAssets() throws Exception {
        String[][] files={
            {"vendor/fonts/Interstate Condensed Regular.otf","e48828f492daeb7571c93be03a2f5b43de832abbe398b7f5fc9e62746b35d675"},
            {"vendor/fonts/Inter-ExtraLight.ttf","81e7bd4155ef35c210a806e08873f421ca4af71ec73cef3cd81a03a78c0b5a6b"}
        };
        for (String[] item:files) {
            MessageDigest hash=MessageDigest.getInstance("SHA-256");
            try (InputStream input=getTargetContext().getAssets().open(item[0])) {
                byte[] block=new byte[8192];int count;while ((count=input.read(block))!=-1) hash.update(block,0,count);
            }
            if (!hex(hash.digest()).equals(item[1])) throw new AssertionError("Packaged font identity: "+item[0]);
        }
    }
    private static void save(Bitmap bitmap,File file) throws Exception {
        try (FileOutputStream output=new FileOutputStream(file)) {
            if (!bitmap.compress(Bitmap.CompressFormat.PNG,100,output)) throw new AssertionError("PNG export");
        }
    }
    private static String pixelsHash(Bitmap bitmap) throws Exception {
        int[] argb=new int[bitmap.getWidth()*bitmap.getHeight()];
        bitmap.getPixels(argb,0,bitmap.getWidth(),0,0,bitmap.getWidth(),bitmap.getHeight());
        ByteBuffer buffer=ByteBuffer.allocate(argb.length*4);
        for (int pixel:argb) buffer.putInt(pixel);
        return hex(MessageDigest.getInstance("SHA-256").digest(buffer.array()));
    }
    private static String hex(byte[] bytes) {
        StringBuilder out=new StringBuilder();for (byte value:bytes) out.append(String.format(java.util.Locale.ROOT,"%02x",value&255));return out.toString();
    }
}
