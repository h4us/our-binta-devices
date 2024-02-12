// TEST

thisProcess.openPorts;
thisProcess.openUDPPort(12000);

o = OSCFunc({| msg | [msg].postln; }, '/acc/x', nil, 12000);
o.free;

OSCdef(\test, {| msg | 'a'.post; msg.postln; }, '/acc/y', nil, 12000);
OSCdef(\test).free;

Ndef(\sound, { |fadeTime = 1, gate = 1|
    var e = Env.adsr(fadeTime, 0.01, 0.4, fadeTime).ar(2, gate);
    SinOsc.ar(100 + (e * 700), SinOsc.ar(208) * (1 - e) * 6) * e * 0.1
}).play

Ndef(\sound).play
Ndef(\sound).set(\gate, 0)
