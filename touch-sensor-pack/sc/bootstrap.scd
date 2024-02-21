Server.local.options.maxLogins = 3;
Server.local.waitForBoot.do({
    Tdef(\bootstrap, {
        1.do({
            5.wait;

            // Server.local.statusWatcher.stopAliveThread;
            // "stopAliveThread - sc3.8.x on RaspberryPi tweaks..".postln;
            // "waiting..".postln;

            5.wait;
            "..start".postln;

            // Do something...

            Ndef(\test, { SinOsc.ar(440) ! 2 });
            Ndef(\test).play;
        });
    }).play;
})
