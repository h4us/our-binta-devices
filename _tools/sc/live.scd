//
ServerOptions.devices.do({| dev |
	dev.postln;
});

Server.default.options.numInputBusChannels

NetAddr("192.168.50.30", 12000).sendMsg(\p_threshold, 3000); // tsukahara
NetAddr("192.168.50.31", 12000).sendMsg(\p_threshold, 3000); // mikajiri
NetAddr("192.168.50.32", 12000).sendMsg(\p_threshold, 3500); // matsumi
NetAddr("192.168.50.33", 12000).sendMsg(\p_threshold, 3550); // naze

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

			if (Server.default.statusWatcher.hasBooted, {
				Ndef(target).set(
					//  \freq, msg[2].pow(1.1 + (msg[1] / 4096)).clip(0, 20000)
					\widerate, (msg[2].pow(1.1 + (msg[1] / 4096)) * 0.0002).clip(-100, 100)
				);
				Ndef(target).set(\amp, msg[1] / 4096);
				if (target2.notNil, { Ndef(target2).set(\amp, 0.4); });
			});
        }, {
            // addr.post; msg.postln;
            // Ndef(target).set(\freq, 50);
			if (Server.default.statusWatcher.hasBooted, {
				Ndef(target).set(\amp, 0);
				if (target2.notNil, { Ndef(target2).set(\amp, 0); });
			});
        });

    }, {
       //  "analog olphan: ".post; (addr.ip ++  " ").post; msg.postln;
    });

}, '/analog', nil, 12000);

// OSCdef(\r1).clear;

OSCdef(\r3, {
    |msg, time, addr, recvPort|
    var target;

    if (true, { target = \r2p4; });

    if (target.notNil, {
        "hsrs: ".post; (addr.ip ++ " ").post; msg.postln;
		ObjectTable.put(\oneshot_buf, [9, 10].choose);
    }, {
        "hsrs olphan: ".post; addr.post; msg.postln;
    });

}, '/btl/ahrs', nil, 12000);

// OSCdef(\r3).clear;

OSCdef(\r4, {
    |msg, time, addr, recvPort|
    var target;

    if (true, { target = \r2p4; });

    if (target.notNil, {
        "shock: ".post; (addr.ip ++ " ").post; msg.postln;
		if (Server.default.statusWatcher.hasBooted && (msg[1] < 0.3), {
			[
			 // { (PlayBuf.ar(1, ObjectTable.at(\oneshot_buf), loop: 0, doneAction: 2) * 0.3).dup },
			{ (FreeVerb.ar(PlayBuf.ar(1, ObjectTable.at(\oneshot_buf), loop: 0, doneAction: 2) * 0.2, room: 20)).dup }
			].choose.play;
        });
    }, {
        "shock olphan: ".post; (addr.ip ++ " ").post; msg.postln;
    });

}, '/btl/shock', nil, 12000);

// OSCdef(\r4).clear;

ObjectTable.put(\oneshot_buf, 9);

//
OSCdef(\pff, {
    |msg, time, addr, recvPort|
    // (20 + (msg[4]*300)).asInt.clip(0,255).postln;
    NetAddr("127.0.0.1", 13000).sendMsg('/all', 255, 255, 255, (20 + msg[4] * 300).asInt.clip(20, 255));
}, '/pf_submix/peak');

// OSCdef(\pff).clear;

//
OSCdef(\amf, {
    |msg, time, addr, recvPort|
    // msg.postln;
	NetAddr("127.0.0.1", 13000).sendMsg('/all', 0, 0, 0, msg[3]);
}, '/pf_submix/reply');

// OSCdef(\amf).clear;

ObjectTable.put(\ruuning, 0);

OSCdef(\gg, {
    |msg, time, addr, recvPort|
    msg.postln;
	ObjectTable.at(\oneshot_buf).postln;

	if (Server.default.statusWatcher.hasBooted && (msg[1] > 8), {
        [
			{ Pan2.ar(
				PlayBuf.ar(1, ObjectTable.at(\oneshot_buf), loop: 0, doneAction: 2) * 0.75,
				EnvGen.kr(Env.linen(0, 0, 1)) * 2 - 1, doneAction: 2
			) }
        ].choose.play;
		"running!".postln;
    });
}, '/shock', nil, 12000);

// OSCdef(\gg).clear;

