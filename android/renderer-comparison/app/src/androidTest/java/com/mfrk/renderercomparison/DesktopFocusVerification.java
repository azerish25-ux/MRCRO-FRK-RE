package com.mfrk.renderercomparison;

import android.app.Instrumentation;
import com.mfrk.uihost.UiMessageQueue;
import org.json.JSONArray;
import org.json.JSONObject;

final class DesktopFocusVerification {
    private DesktopFocusVerification() {}
    static JSONObject verify(Instrumentation instrumentation) throws Exception {
        String[] names={"reverse_order","deleted_focus","redirect_snapshot","remove_earlier","clear_list","append_retrigger"};
        int[][] expected={{2,0,1,0,0,0},{2,0,1,-1,0,-1},{2,0,1,0,0,0,2,1,1,1,0,1},{2,0,2,0,1,0},{2,0},{2,0,1,0,0,0,3,0,2,0,1,0,0,0}};
        JSONArray cases=new JSONArray();
        for(int mode=0;mode<names.length;++mode) {
            final int scenario=mode;long[] handle={0};int[][] state={null};
            try {
                instrumentation.runOnMainSync(() -> {UiMessageQueue.initialize();handle[0]=NativeDesktopFocusProbe.start(scenario);});
                instrumentation.waitForIdleSync();
                instrumentation.runOnMainSync(() -> state[0]=NativeDesktopFocusProbe.snapshot(handle[0]));
                int[] values=state[0],events=expected[mode];
                if(values.length!=5+events.length||values[0]!=events.length/2||values[1]!=0||values[2]!=1||values[3]!=0||values[4]!=0)
                    throw new AssertionError("Desktop focus dispatch: "+names[mode]);
                for(int i=0;i<events.length;++i)if(values[5+i]!=events[i])throw new AssertionError("Desktop focus snapshot/order: "+names[mode]);
                cases.put(new JSONObject().put("name",names[mode]).put("state",new JSONArray(values)));
            } finally {
                if(handle[0]!=0)instrumentation.runOnMainSync(() -> NativeDesktopFocusProbe.close(handle[0]));
            }
        }
        return new JSONObject().put("passed",true).put("cases",cases)
            .put("scope","actual Desktop focus target over Android Handler/JNI; listeners are debug observers, full scene remains unfinished");
    }
}
