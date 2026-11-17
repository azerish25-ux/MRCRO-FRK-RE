package com.mfrk.renderercomparison;

import android.app.Instrumentation;
import com.mfrk.uihost.UiMessageQueue;
import org.json.JSONArray;
import org.json.JSONObject;

final class UiQueueVerification {
    private UiQueueVerification() {}
    static JSONObject verify(Instrumentation instrumentation) throws Exception {
        String[] names={"worker_fifo","expired_owner","expired_listener","cancelled","nested_post","heap_payload"};
        JSONArray cases=new JSONArray();
        for (int scenario=0;scenario<names.length;++scenario) {
            final int mode=scenario;
            long[] handle={0};
            try {
                instrumentation.runOnMainSync(() -> {
                    UiMessageQueue.initialize();
                    handle[0]=NativeUiQueueProbe.create(mode==4);
                    long message=NativeUiQueueProbe.postFromWorker(handle[0],mode==0?3:1,mode==5);
                    if (mode==1 || mode==2) NativeUiQueueProbe.expire(handle[0],mode==2);
                    if (mode==3 && !NativeUiQueueProbe.cancel(message)) throw new AssertionError("Pending UI message cancellation");
                });
                instrumentation.waitForIdleSync();
                int[] state=NativeUiQueueProbe.snapshot(handle[0]);
                int expected=mode==0?3:mode==4?2:mode==5?1:0;
                if (state.length!=9 || state[0]!=expected || state[1]!=0 || state[2]!=(expected==0?0:1))
                    throw new AssertionError("UI queue delivery/lifetime: "+names[scenario]);
                if (mode==0 && (state[3]!=0x3e000000 || state[4]!=0x3e800000 || state[5]!=0x3f000000))
                    throw new AssertionError("Worker UI queue order");
                if (mode==4 && (state[3]!=0x3e000000 || state[4]!=0x3f000000))
                    throw new AssertionError("Nested UI post order");
                if (mode==5 && (state[3]!=0x58473625 || state[7]!=2 || state[8]!=24))
                    throw new AssertionError("Owned UI heap payload");
                cases.put(new JSONObject().put("name",names[scenario]).put("state",new JSONArray(state)));
            } finally {
                if (handle[0]!=0) instrumentation.runOnMainSync(() -> NativeUiQueueProbe.close(handle[0]));
            }
        }
        return new JSONObject().put("passed",true).put("cases",cases)
            .put("scope","real Android Handler/JNI transport and typed ownership; observer recipient, not scene/audio parity");
    }
}
