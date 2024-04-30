//
/* TODO: modular */
//
// Server.killAll;

(
Server.default.options.device = "ASIO : MOTU Gen 5";
Server.default.options.sampleRate = 48000;
Server.default.options.numOutputBusChannels = 4;
Server.default.options.numInputBusChannels = 20;
Server.local.boot.makeGui;
NdefMixer(Server.local);
// Server.local.scope(2);
)

(
//
/* sampler test */
//

Spec.add(\bufnum, ControlSpec(0, 200, \lin, 1, 0));
Spec.add(\busindex, ControlSpec(0, 511, step: 1));
Spec.add(\ioindex, ControlSpec(0, Server.default.options.numInputBusChannels - 1, step: 1));
Spec.add(\loopgate, ControlSpec(-10, 10, \lin));
Spec.add(\startPos, \unipolar);
Spec.add(\endPos, \unipolar);
Spec.add(\widerate, ControlSpec(-100, 100, \lin));

Server.local.waitForBoot.do({
    var filer = Array();
    var dir, win, sfview, btn, lview, fn, info, sender, layout;
    var ww = 1080;
    var wh = 240;

    var default_sf = File.readAllString(Document.current.dir ++ "/config.json").parseJSON.at("soundfiles");
    var privateBusIndex = Server.default.options.numOutputBusChannels + Server.default.options.numInputBusChannels;
    var submixIndex = privateBusIndex + 100;

    default_sf.do({|path, pathidx|
        filer = filer ++ SoundFile.collect(path);
    });
    dir = IdentityDictionary.new;
    win = Window("files", Rect(0, 0, ww, wh))
    .onClose_({
        "close".postln;
        dir.do({|el|
            el.at(\bufnum).postln;
            Server.default.sendMsg("/b_free", el.at(\bufnum));
            filer.at(el.at(\index)).close;
        });
    })
    .front;
    win.addFlowLayout;

    //
    sfview = SoundFileView(win, 440@wh)
    .mouseUpAction_({|self|
        self.selections[self.currentSelection].postln;
        if(dir.at(self.soundfile.path.asSymbol).at(\buffered) == 1, {
            ">>" ++ dir.at(self.soundfile.path.asSymbol).at(\bufnum).postln;
        });
    });

    //
    fn = {|target, toggle|
        var bn = nil;
        var sf = nil;
        dir.at(target.asSymbol).put(\buffered, toggle);
        //
        if(dir.at(target.asSymbol).at(\buffered) == 1, {
            sf = filer.at(dir.at(target.asSymbol).at(\index));
            sf.postln;
            sf.openRead(sf.path);
            dir.at(target.asSymbol).put(\bufnum, sf.asBuffer(Server.local).bufnum);
            dir.at(target.asSymbol).postln;
        }, {
            "to free".postln;
            dir.at(target.asSymbol).postln;
        });
    };

    filer.do({|el, i|
        dir.put(el.path.asSymbol, (index:i, buffered: 0));
        fn.value(el.path, 1);
    });

    sfview.soundfile = filer.at(0);
    sfview.read(0, filer.at(0).numFrames);

    //
    lview = ListView(win, 300@wh)
    .items_(filer.collect({|el| el.path}))
    .action_({|index|
        filer.collect({|el| el.path}).at(index.value).postln;
        sfview.soundfile = filer.at(index.value);
        sfview.read(0, filer.at(index.value).numFrames);
        sfview.refresh;
        btn.value = dir.at(filer.at(index.value).path.asSymbol).at('buffered');
        info.value = dir.at(filer.at(index.value).path.asSymbol);
    });
    layout = VLayoutView(win, 300@wh);
    btn = Button(layout, 300@30)
    .states_([
        ["use buffer", Color.black, Color.red],
        ["buffered", Color.red, Color.white],
    ])
    .action_({|self|
        var sf = nil;
        self.value.postln;
        fn.value(lview.items.at(lview.value), self.value);
    });
    info = EZText(layout, 300@40, "info:", {|ez| ez.postln;}, ["-"], true);
    sender = EZText(layout, 300@40, "sendTo:", {|ez|
        dir.at(lview.items.at(lview.value).asSymbol).at(\bufnum).postln;
        ObjectTable.put(\oneshot_buf, dir.at(lview.items.at(lview.value).asSymbol).at(\bufnum));
        // Ndef(ez.value.at(0).asSymbol).set(\bufnum, dir.at(lview.items.at(lview.value).asSymbol).at(\bufnum));
        ez.value.postln;
    }, ["-"], true);

    4.do({| i |
        //
        Ndef(("r1p"++(i+1)).asSymbol, {|amp = 0.5, bufnum = 0, widerate = 1, startPos = 0, endPos = 1,  gain = 1, preamp = 1, pan = 0|
            var bo = BufRd.ar(
                1, bufnum,
                Phasor.ar(0, BufRateScale.kr(bufnum) * widerate, BufFrames.kr(bufnum) * startPos, BufFrames.kr(bufnum) * endPos),
                1, 4
            );
            Pan2.ar(bo * amp * VarLag.kr(preamp, 0.5) * gain, pan);
        });
        Ndef(("r1p"++(i+1)).asSymbol).play(privateBusIndex);
        Ndef(("r1p"++(i+1)).asSymbol).set(\bufnum, dir.at(filer.at(i).path.asSymbol).at(\bufnum), \amp, 0);
    });

    [50, 60, 70, 80].do({| f, idx |
        Ndef(("subase"++(idx + 1)).asSymbol, {| freq = 50, amp = 0.4, gain = 0.5, pan = 0 |
            // SinOsc.ar(freq) * gain * Slew.kr(amp, 0.01, 0.01) ! 2;
            Pan2.ar(SinOsc.ar(freq) * gain * amp, pan);
        });
        Ndef(("subase"++(idx + 1)).asSymbol).play(privateBusIndex);
        Ndef(("subase"++(idx + 1)).asSymbol).set(\freq, f, \amp, 0);
    });

    4.do({| idx |
        Ndef(("pf"++(idx + 1)).asSymbol, {| ioindex = 0, amp = 1.0 |
            var ioin = SoundIn.ar(ioindex);
            var rmsd = SendPeakRMS.ar(ioin, cmdName: '/pf'++(idx+1));
            ioin * amp;
            // Silent.ar;
        });
        Ndef(("pf"++(idx + 1)).asSymbol).play(submixIndex);
        Ndef(("pf"++(idx + 1)).asSymbol).set(\ioindex, idx);
    });

    ObjectTable.put(\oneshot_buf, 10);
    ObjectTable.at(\oneshot_buf);

    Routine({
        3.wait;

        Ndef(\outmix, {|gain = 1, amp = 1|
            In.ar(privateBusIndex, 2) * gain * amp;
        });
        Ndef(\outmix).play;

        Ndef(\inmix, {|gain = 1, amp = 1, busindex = 0|
            var submixin = In.ar(busindex) * gain * amp;
            var rmsd = SendPeakRMS.ar(submixin, 60.0, 2, cmdName: '/pf_submix');
            submixin;
        });
        Ndef(\inmix).play(submixIndex + 2);
        Ndef(\inmix).set(\ioindex, submixIndex);

        4.do({|i|
            Ndef(("r1p"++(i+1)).asSymbol).pause;
            Ndef(("subase"++(i + 1)).asSymbol).pause;
        })
    }).play;
});
);
