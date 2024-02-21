//
/* TODO: modular */
//
Server.local.boot.makeGui;
NdefMixer(Server.local);
Server.local.scope(2);

(
//
/* sampler test */
//

Spec.add(\bufnum, ControlSpec(0, 200, \lin, 1, 0));
Spec.add(\busindex, ControlSpec(0, 99, step:1));
Spec.add(\loopgate, ControlSpec(-10, 10, \lin));
Spec.add(\startPos, \unipolar);
Spec.add(\endPos, \unipolar);
Spec.add(\widerate, ControlSpec(-5, 5, \lin));

Server.local.waitForBoot.do({
    var filer = Array();
    var dir, win, sfview, btn, lview, fn, info, sender, layout;
    var ww = 1080;
    var wh = 240;

    var default_sf = File.readAllString("config.json".standardizePath).parseJSON.at("soundfiles");

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
        Ndef(ez.value.at(0).asSymbol).set(\bufnum, dir.at(lview.items.at(lview.value).asSymbol).at(\bufnum));
    }, ["-"], true);

    4.do({|i|
        Ndef(("player_0_"++i).asSymbol, {|amp = 0.5, bufnum = 0, widerate = 1, startPos = 0, endPos = 1,  gain = 1, preamp = 1, pan = 0|
            var bo = BufRd.ar(
                1, bufnum,
                Phasor.ar(0, BufRateScale.kr(bufnum) * widerate, BufFrames.kr(bufnum) * startPos, BufFrames.kr(bufnum) * endPos),
                1, 4
            );
            Pan2.ar(bo * amp * VarLag.kr(preamp, 0.5) * gain, pan);
        });
        Ndef(("player_0_"++i).asSymbol).play(10);
        Ndef(("player_0_"++i).asSymbol).set(\bufnum, dir.at(filer.at(i).path.asSymbol).at(\bufnum));
    });

    Routine({
        3.wait;
        Ndef(\mbus, {|gain = 1, amp = 1|
            [In.ar(10) * gain * amp, In.ar(11) * gain * amp];
        });

        Ndef(\mbus).play;
        4.do({|i|
            Ndef(("player_0_"++i).asSymbol).pause;
        })
    }).play;
});
);
