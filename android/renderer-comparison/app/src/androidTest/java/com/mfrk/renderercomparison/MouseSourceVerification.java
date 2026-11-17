package com.mfrk.renderercomparison;

import android.app.Instrumentation;
import com.mfrk.uihost.UiMessageQueue;
import org.json.JSONArray;
import org.json.JSONObject;

final class MouseSourceVerification {
    private MouseSourceVerification() {}
    static JSONObject verify(Instrumentation instrumentation) throws Exception {
        String[] names={"forced_refresh","below_drag_threshold","at_drag_threshold","deleted_target","target_transition","clock_regression"};
        int[][] expected={{1,0,0,0,1,1,0,0,1000,0,0},{0,1,0,0,0,1,0,0,1000,0,0},{0,1,0,0,0,1,0,0,1000,1,0},{0,0,0,0,1,1,0,0,0,0,0},{1,0,1,1,1,2,0,0,1000,0,0},{1,0,0,0,1,1,0,0,1500,0,0}};
        JSONArray cases=new JSONArray();
        for(int mode=0;mode<names.length;++mode){
            final int scenario=mode;long[] handle={0};int[][] state={null};
            try{
                instrumentation.runOnMainSync(() -> {UiMessageQueue.initialize();handle[0]=NativeMouseSourceProbe.start(scenario);});
                instrumentation.waitForIdleSync();
                instrumentation.runOnMainSync(() -> state[0]=NativeMouseSourceProbe.snapshot(handle[0]));
                int[] values=state[0];if(values.length!=expected[mode].length)throw new AssertionError("Mouse source state size");
                for(int i=0;i<values.length;++i)if(values[i]!=expected[mode][i])throw new AssertionError("Mouse source dispatch: "+names[mode]+" "+new JSONArray(values));
                cases.put(new JSONObject().put("name",names[mode]).put("state",new JSONArray(values)));
            }finally{if(handle[0]!=0)instrumentation.runOnMainSync(() -> NativeMouseSourceProbe.close(handle[0]));}
        }
        return new JSONObject().put("passed",true).put("cases",cases).put("scope","real queued source state owner; hit/coordinate/event services are observers, full scene remains unfinished");
    }
}
