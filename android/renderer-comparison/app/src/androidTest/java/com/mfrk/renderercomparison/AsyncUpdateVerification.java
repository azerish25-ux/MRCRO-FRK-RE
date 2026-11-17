package com.mfrk.renderercomparison;

import android.app.Instrumentation;
import com.mfrk.uihost.UiMessageQueue;
import org.json.JSONArray;
import org.json.JSONObject;

final class AsyncUpdateVerification {
    private AsyncUpdateVerification() {}
    static JSONObject verify(Instrumentation instrumentation) throws Exception {
        String[] names={"coalesced","cancelled","destroyed_owner","nested","cancel_retrigger","new_owner","self_destroy","worker_coalesced"};
        JSONArray cases=new JSONArray();
        for (int mode=0;mode<names.length;++mode) {
            final int scenario=mode;
            long[] handle={0};int[][] state={null};
            try {
                instrumentation.runOnMainSync(() -> { UiMessageQueue.initialize();handle[0]=NativeAsyncUpdateProbe.start(scenario); });
                instrumentation.waitForIdleSync();
                instrumentation.runOnMainSync(() -> state[0]=NativeAsyncUpdateProbe.snapshot(handle[0]));
                int[] values=state[0];int expected=mode==1||mode==2?0:mode==3?2:1;
                if (values.length!=7||values[0]!=expected||values[1]!=0||values[2]!=(expected==0?0:1)||values[3]!=0||values[4]!=0)
                    throw new AssertionError("Async update delivery: "+names[mode]);
                if (values[5]!=(mode==2||mode==6?0:1)||values[6]!=(mode==5?1:0))
                    throw new AssertionError("Async update owner lifetime: "+names[mode]);
                cases.put(new JSONObject().put("name",names[mode]).put("state",new JSONArray(values)));
            } finally {
                if (handle[0]!=0) instrumentation.runOnMainSync(() -> NativeAsyncUpdateProbe.close(handle[0]));
            }
        }
        return new JSONObject().put("passed",true).put("cases",cases)
            .put("scope","real main-loop async delivery and focus-word ownership; callback observer, full scene remains unfinished");
    }
}
