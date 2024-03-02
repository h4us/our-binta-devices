Server.local.boot;
Server.local.makeGui;
Server.local.scope;
Server.killAll;

thisProcess.openPorts; // list all open ports
thisProcess.openUDPPort(12000); // attempt to open 1121

//
NetAddr("192.168.50.30", 12000).sendMsg(\p_threshold, 1800); // tsukahara
NetAddr("192.168.50.31", 12000).sendMsg(\p_threshold, 1000); // mikajiri
NetAddr("192.168.50.32", 12000).sendMsg(\p_threshold, 2000); // matsumi
NetAddr("192.168.50.33", 12000).sendMsg(\p_threshold, 1000); // naze

// Ndef(\r1p1, {| freq = 440, amp = 0.5, bwr = 0.8, bufnum = 0 |
//     // Resonz.ar(LFNoise0.ar(3000), Lag.kr(freq, 0.1), bwr) ! 2;
//     // Resonz.ar(LFNoise0.ar(3000), Slew.kr(freq, 2000, 2000), bwr) * amp ! 2;
//     //
//     PlayBuf.ar(3, bufnum, Slew.kr(freq, 2000, 2000) / 3000, loop: 1, doneAction: 0) * amp ! 2;
// });
// Ndef(\r1p1).play(10);
// Ndef(\r1p1).pause;

// Ndef(\r1p2, {| freq = 440, amp = 0.5, bwr = 0.8, bufnum = 1 |<
//     // Resonz.ar(LFNoise0.ar(3000), Lag.kr(freq, 0.1), bwr) ! 2;
//     // Resonz.ar(LFNoise0.ar(3000), Slew.kr(freq, 2000, 2000), bwr) * amp ! 2;
//     //
//     PlayBuf.ar(3, bufnum, Slew.kr(freq, 2000, 2000) / 3000, loop: 1, doneAction: 0) * amp ! 2;
// });
// Ndef(\r1p2).play(10);
// Ndef(\r1p2).pause;

// Ndef(\r1p3, {| freq = 440, amp = 0.5, bwr = 0.8, bufnum = 2 |
//     // Resonz.ar(LFNoise0.ar(3000), Lag.kr(freq, 0.1), bwr) ! 2;
//     // Resonz.ar(LFNoise0.ar(3000), Slew.kr(freq, 2000, 2000), bwr) ! 2;
//     // Resonz.ar(LFNoise0.ar(3000), freq, bwr) ! 2;
//     // PlayBuf.ar(1, f, Slew.kr(freq / 1000, 1, 1), loop: 1, doneAction: 0) * amp ! 2;
//     // Resonz.ar(LFNoise0.ar(3000), Slew.kr(freq, 2000, 2000), bwr) * amp ! 2;
//     PlayBuf.ar(3, bufnum, Slew.kr(freq, 2000, 2000) / 3000, loop: 1, doneAction: 0) * amp ! 2;
// });
// Ndef(\r1p3).play(10);
// Ndef(\r1p3).pause;


OSCdef(\r1, {
    |msg, time, addr, recvPort|
    var target, target2;

    if (addr.ip == "192.168.50.30", { target = \r1p1; target2 = \subase1 });
    if (addr.ip == "192.168.50.31", { target = \r1p2; target2 = \subase2 });
    if (addr.ip == "192.168.50.32", { target = \r1p3; target2 = \subase3 });
    if (addr.ip == "192.168.50.33", { target = \r1p4; target2 = \subase4 });

    if (target.notNil, {
        // addr.post; msg.postln;
        if (msg[2] > 0, {
			"ananlog: ".post; (addr.ip ++ " ").post; msg.postln;
            Ndef(target).set(
                //  \freq, msg[2].pow(1.1 + (msg[1] / 4096)).clip(0, 20000)
                \widerate, (msg[2].pow(1.1 + (msg[1] / 4096)) * 0.0005).clip(-100, 100)
            );
            Ndef(target).set(\amp, msg[1] / 4096);
            if (target2.notNil, { Ndef(target2).set(\amp, 0.4); });
        }, {
            // addr.post; msg.postln;
            // Ndef(target).set(\freq, 50);
            Ndef(target).set(\amp, 0);
            if (target2.notNil, { Ndef(target2).set(\amp, 0); });
        });

    }, {
		"analog olphan: ".post; (addr.ip ++  " ").post; msg.postln;
    });

}, '/analog', nil, 12000);

OSCdef(\r1).clear;

ObjectTable.put(\oneshot_buf,);
ObjectTable.at(\oneshot_buf);

OSCdef(\r3, {
    |msg, time, addr, recvPort|
    var target;

    if (addr.ip == "192.168.50.73", { target = \r2p4; });

    if (target.notNil, {
        addr.post; msg.postln;
        // { (PlayBuf.ar(1, 10, loop: 0, doneAction: 2) * 0.2).dup }.play;
        Ndef(\oneshot,{ | trig = 1 | (PlayBuf.ar(1, ObjectTable.at(\oneshot_buf), trigger: trig, loop: 0, doneAction: 2) * 0.2).dup });
        Ndef(\oneshot);

    }, {
        "hsrs olphan: ".post; addr.post; msg.postln;
    });

}, '/btl/ahrs', nil, 12000);

OSCdef(\r4, {
    |msg, time, addr, recvPort|
    var target;

    if (true, { target = \r2p4; });

    if (target.notNil, {
		"shock: ".post; (addr.ip ++ " ").post; msg.postln;
        if (msg[1] < 0.4, {
			{ (PlayBuf.ar(1, ObjectTable.at(\oneshot_buf), loop: 0, doneAction: 2) * 0.2).dup }.play;
			/*
            Ndef(\shot, {| amp = 0.2 |
                Pan2.ar(PlayBuf.ar(1, ObjectTable.at(\oneshot_buf), loop: 0, doneAction: 2) * amp, 0)
            });
			Ndef(\shot).play(12);
			*/

        });
    }, {
		"shock olphan: ".post; (addr.ip ++ " ").post; msg.postln;
    });

}, '/btl/shock', nil, 12000);

OSCdef(\r4).clear;