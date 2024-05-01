
Server.local.boot;
Server.local.makeGui;
Server.local.scope;
Server.killAll;

thisProcess.openPorts; // list all open ports
thisProcess.openUDPPort(12000); // attempt to open 1121

a = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/m1.wav");
b = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/m2.wav");
c = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/m3.wav");
d = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/m4.wav");
e = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/m5.wav");
f = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/mjk/1.wav");
g = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/mjk/2.wav");
h = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/mjk/3.wav");
i = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/mjk/4.wav");
j = Buffer.read(Server.local, "/home/inafact/Dropbox/Public/gonzo-binta/naze_sound/mjk/5.wav");

/*
{ (PlayBuf.ar(1, b, loop: 0) * 0.5).dup }.play;
{ (PlayBuf.ar(1, c, loop: 0) * 0.5).dup }.play;
{ (PlayBuf.ar(1, d, loop: 0) * 0.5).dup }.play;
{ (PlayBuf.ar(1, e, loop: 0) * 0.5).dup }.play;
{ (PlayBuf.ar(1, f, loop: 0) * 0.5).dup }.play;
{ (PlayBuf.ar(1, g, loop: 0) * 0.5).dup }.play;
{ (PlayBuf.ar(1, h, loop: 0) * 0.5).dup }.play;
*/

OSCdef(\r0, {
    |msg, time, addr, recvPort|
    msg.postln;
    if ((msg[1] <= 120), {
        addr.post; msg.postln;

        // if (addr.ip == "192.168.11.4", {
            [
                // { Crackle.ar((msg[1] * 0.1)) * EnvGen.kr(Env.perc(0.01, 0.1), doneAction: 2)},
                { (PlayBuf.ar(1, a, loop: 0, doneAction: 2) * 0.3).dup },
                { (PlayBuf.ar(1, b, loop: 0, doneAction: 2) * 0.3).dup },
                // { (PlayBuf.ar(1, c, loop: 0, doneAction: 2) * 0.3).dup },
                // { (PlayBuf.ar(1, d, loop: 0, doneAction: 2) * 0.3).dup },
                // { (PlayBuf.ar(1, e, loop: 0, doneAction: 2) * 0.3).dup },
                // { (PlayBuf.ar(1, f, loop: 0) * 0.3).dup }
            ].choose().play;
        // });

        // if (addr.ip == "192.168.11.6", {
        //     [
        //         { LFPulse.ar(msg[1] * 0.2) * EnvGen.kr(Env.perc(0.001, 0.01), doneAction: 2)},
        //         { (PlayBuf.ar(1, d, loop: 0) * 0.3).dup },
        //         { (PlayBuf.ar(1, e, loop: 0) * 0.3).dup },
        //         { (PlayBuf.ar(1, f, loop: 0) * 0.3).dup }
        //     ].choose().play;
        // });

        // if (addr.ip == "192.168.11.7", {
        //     [
        //         { Crackle.ar((msg[1] * 0.1)) * EnvGen.kr(Env.perc(0.01, 0.1), doneAction: 2)},
        //         { LFPulse.ar(msg[1] * 0.2) * EnvGen.kr(Env.perc(0.001, 0.01), doneAction: 2)},
        //         { (PlayBuf.ar(1, g, loop: 0) * 0.3).dup },
        //         { (PlayBuf.ar(1, h, loop: 0) * 0.3).dup }
        //     ].choose().play;
        // });
    })

}, '/shock', nil, 12000);

OSCdef(\r0).free



Ndef(\r1p1, {| freq = 440, amp = 0.5 |
	SinOsc.ar(freq) ! 2;
});

Ndef(\subase, {| freq = 50, amp = 0.4, gain = 0.5 |
    // SinOsc.ar(freq) * gain * Slew.kr(amp, 0.01, 0.01) ! 2;
    SinOsc.ar(freq) * gain * amp ! 2;
});

Ndef(\subase).gui;
Ndef(\subase).play;

Ndef(\subase2, {| freq = 60, amp = 0.4, gain = 0.5 |
    // SinOsc.ar(freq) * gain * Slew.kr(amp, 0.01, 0.01) ! 2;
    SinOsc.ar(freq) * gain * amp ! 2;
});
Ndef(\subase2).gui;
Ndef(\subase2).play;


Ndef(\r1p1).gui;
Ndef(\r1p1).play;

Ndef(\r1p2, {| freq = 440, amp = 0.5 |
	SinOsc.ar(freq) ! 2;
});

Ndef(\r1p2).gui;
Ndef(\r1p2).play;

Ndef(\r1p3, {| freq = 440, amp = 0.5, bwr = 0.8 |
    // Resonz.ar(LFNoise0.ar(3000), Lag.kr(freq, 0.1), bwr) ! 2;
    // Resonz.ar(LFNoise0.ar(3000), Slew.kr(freq, 2000, 2000), bwr) ! 2;
    // Resonz.ar(LFNoise0.ar(3000), freq, bwr) ! 2;
    // PlayBuf.ar(1, f, Slew.kr(freq / 1000, 1, 1), loop: 1, doneAction: 0) * amp ! 2;
    // Resonz.ar(LFNoise0.ar(3000), Slew.kr(freq, 2000, 2000), bwr) * amp ! 2;
    PlayBuf.ar(3, a, Slew.kr(freq, 2000, 2000) / 3000, loop: 1, doneAction: 0) * amp ! 2;
});


Ndef(\r1p3).gui;
Ndef(\r1p3).play;
// Ndef(\r1p3).stop;

Ndef(\r1p4, {| freq = 440, amp = 0.5, bwr = 0.8 |
    // Resonz.ar(LFNoise0.ar(3000), Lag.kr(freq, 0.1), bwr) ! 2;
    // Resonz.ar(LFNoise0.ar(3000), Slew.kr(freq, 2000, 2000), bwr) * amp ! 2;
    //
    PlayBuf.ar(3, a, Slew.kr(freq, 2000, 2000) / 3000, loop: 1, doneAction: 0) * amp ! 2;
});

Ndef(\r1p4).gui;
Ndef(\r1p4).play;
Ndef(\r1p4).clear;

// Ndef(\r1p4).stop;


OSCdef(\r1, {
    |msg, time, addr, recvPort|
    var target, target2;

    if (addr.ip == "192.168.20.3", { target = \r1p4; target2 = \subase });
    if (addr.ip == "192.168.20.64", { target = \r1p3; target2 = \subase2 });

    if (target.notNil, {
        if (msg[2] > 0, {
            // addr.post; msg.postln;
            Ndef(target).set(
                \freq, msg[2].pow(1.1 + (msg[1] / 4096)).clip(0, 20000)
            );
            Ndef(target).set(\amp, msg[1] / 4096);
            if (target2.notNil, { Ndef(target2).set(\amp, 0.4); });
        }, {
            // addr.post; msg.postln;
            Ndef(target).set(\freq, 50);
            Ndef(target).set(\amp, 0);
            if (target2.notNil, { Ndef(target2).set(\amp, 0); });
        });

    }, {
        "olphan: ".post; addr.post; msg.postln;
    });

}, '/analog', nil, 12000);


OSCdef(\r2, {
    |msg, time, addr, recvPort|
    var target;

    if (addr.ip == "192.168.20.4", { target = \r2p4; });
    if (addr.ip == "192.168.20.2", { target = \r2p3; });

    if (target.notNil, {
        addr.post; msg.postln;
    }, {
        "gyro olphan: ".post; addr.post; msg.postln;
    });

}, '/btl/gyro', nil, 12000);

OSCdef(\r3, {
    |msg, time, addr, recvPort|
    var target;

    if (addr.ip == "192.168.20.4", { target = \r2p4; });
    if (addr.ip == "192.168.20.6", { target = \r2p3; });

    if (target.notNil, {
        addr.post; msg.postln;
        { (PlayBuf.ar(1, a, loop: 0, doneAction: 2) * 0.3).dup }.play;
    }, {
        "hsrs olphan: ".post; addr.post; msg.postln;
    });

}, '/btl/ahrs', nil, 12000);



OSCdef(\r1, {
	|msg, time, addr, recvPort|

    if (addr.ip == "192.168.11.3", {
        // addr.ip.post; " | ".post; msg.postln;
		if (msg[1] > 57, {
			Ndef(\r1p1).set(\freq, msg[1]**1.01, \amp, 0.4);
		}, {
			Ndef(\r1p1).set(\freq, msg[1], \amp, 0);
		});
	});

    if (addr.ip == "192.168.11.5", {
		addr.ip.post; " | ".post; msg.postln;
		if (msg[1] > 57, {
			Ndef(\r1p2).set(\freq, msg[1]**1.01, \amp, 0.4);
		}, {
			Ndef(\r1p2).set(\freq, msg[1], \amp, 0);
		});
	});

    if (addr.ip == "192.168.11.8", {
		// addr.ip.post; " | ".post; msg.postln;
		if (msg[1] > 57, {
			Ndef(\r1p3).set(\freq, msg[1]**1.2, \amp, 0.4);
		}, {
			Ndef(\r1p3).set(\freq, msg[1], \amp, 0);
		});
	});

}, '/peaks', nil, 12001);

OSCdef(\r1).free

Ndef(\r1p3).fadeTime=0.01

OSCdef(\test2, {
	|msg, time, addr, recvPort|
	// msg.postln;
	// addr.postln;

	if ((msg[1] >= 7000), {
		msg.postln;
		addr.postln;

		/*
		[
			{ WhiteNoise.ar * EnvGen.kr(Env.perc(0.01, 0.8), doneAction: 2)},
			{ LFPulse.ar(2000) * EnvGen.kr(Env.perc(0.001, 0.1), doneAction: 2)},
			//{ WhiteNoise.ar * EnvGen.kr(Env.perc(0.01, 0.2))}
		].choose().play;
		*/
	})<

}, '/acc/x', nil, 12001);


OSCdef(\test, {
	|msg, time, addr, recvPort|

	addr.postln;
	msg.postln;
}, '/shock', nil, 12000);


OSCdef(\test).free


OSCdef(\test, {
	|msg, time, addr, recvPort|

	addr.postln;
	msg.postln;
}, '/shock', nil, 12000);


o = OSCFunc({
	arg msg, time, addr, recvPort;
	[msg, time, addr, recvPort].postln;
	//(msg[1].abs * 1000).postln;
	// { SinOsc.ar(440) * EnvGen.kr(Env.perc)}.play;
	// Ndef(\test).xset(\freq, msg[1].abs * 100);

}, '/graph/position', nil, 12000);

Ndef(\test, { |freq = 400| SinOsc.ar(freq) ! 2});

o.free;

Ndef(\vx, { |amp = 1| K2A.ar(amp) });
Ndef(\vx).play;
Ndef(\vx).gui;
Ndef(\vx).scope;

Ndef(\vy, { |amp = 1| K2A.ar(amp) });
Ndef(\vy).play;
Ndef(\vy).gui;
Ndef(\vy).scope;

Ndef(\vz, { |amp = 1| K2A.ar(amp) });
Ndef(\vz).play;
Ndef(\vz).gui;
Ndef(\vz).scope;


j = OSCFunc({
	arg msg, time, addr, recvPort;
	[msg, time, addr, recvPort].postln;
	// (msg[1].abs * 1000).postln;

	{ SinOsc.ar(440) * EnvGen.kr(Env.perc)}.play;
	// Ndef(\test2).xset(\freq, msg[1] % 2);
}, '/touch/ch0', n);

j.free;

k = OSCFunc({
	arg msg, time, addr, recvPort;
	[msg, time, addr, recvPort].postln;
	// (msg[1].abs * 1000).postln;

	{ SinOsc.ar(880) * EnvGen.kr(Env.perc)}.play;
	// Ndef(\test2).xset(\freq, msg[1] % 2);
}, '/touch/ch8', n);

k.free;

l = OSCFunc({
	arg msg, time, addr, recvPort;
	[msg, time, addr, recvPort].postln;
	// (msg[1].abs * 1000).postln;

	//{ SinOsc.ar(880) * EnvGen.kr(Env.perc)}.play;
	// Ndef(\test2).xset(\freq, msg[1] % 2);
}, '/analog', n);

l.free;


Ndef(\test, { |freq = 440| SinOsc.ar(freq) ! 2; });
Ndef(\test).play

Ndef(\test).xset(\freq, 20)

Ndef(\test2, { |freq = 440, decay = 0| (SinOsc.ar(freq * decay))) ! 2; });
Ndef(\test2).play
