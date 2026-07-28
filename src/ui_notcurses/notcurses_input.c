#define _XOPEN_SOURCE 700

#include <locale.h>
#include <notcurses/notcurses.h>

char *notcurses_key_str(unsigned int, char *);

int main(void) {
    setlocale(LC_ALL, "");

    struct notcurses_options opts = {
        .flags = NCOPTION_NO_QUIT_SIGHANDLERS,
    };

    struct notcurses *nc = notcurses_init(&opts, NULL);
    if (!nc)
        return 1;

    notcurses_mice_enable(nc, NCMICE_ALL_EVENTS);

    struct ncplane *stdn = notcurses_stdplane(nc);
    ncplane_putstr(stdn, "Click anywhere or press 'q' to quit...");
    notcurses_render(nc);

    ncinput ni;
    uint32_t id;
    bool running = true;
    char kstr[32];

    while (running) {
        id = notcurses_get(nc, NULL, &ni);

        ncplane_erase(stdn);

        if (id == 'q' || id == 'Q') {
            running = false;
        } else if (id == NCKEY_BUTTON1) {
            ncplane_printf(stdn, "Left Click at: X=%d, Y=%d", ni.x, ni.y);
        } else if (id == NCKEY_BUTTON2) {
            ncplane_printf(stdn, "Button Click at: X=%d, Y=%d", ni.x, ni.y);
        } else if (id == NCKEY_RESIZE) {
            ncplane_printf(stdn, "Terminal resized!");
        } else {
            ncplane_printf(stdn, "Key pressed: %s (Code: %u)", notcurses_key_str(id, kstr), id);
        }

        ncplane_putstr_yx(stdn, 2, 0, "Press 'q' to exit.");
        notcurses_render(nc);
    }

    notcurses_mice_disable(nc);
    notcurses_stop(nc);
    return 0;
}
char *notcurses_key_str(unsigned int key_id, char *kstr) {
    switch (key_id) {
    case NCKEY_INVALID:
        strcpy(kstr, "NCKEY_INVALID");
        break;
    case NCKEY_RESIZE:
        strcpy(kstr, "NCKEY_RESIZE");
        break;
    case NCKEY_UP:
        strcpy(kstr, "NCKEY_UP");
        break;
    case NCKEY_RIGHT:
        strcpy(kstr, "NCKEY_RIGHT");
        break;
    case NCKEY_DOWN:
        strcpy(kstr, "NCKEY_DOWN");
        break;
    case NCKEY_LEFT:
        strcpy(kstr, "NCKEY_LEFT");
        break;
    case NCKEY_INS:
        strcpy(kstr, "NCKEY_INS");
        break;
    case NCKEY_DEL:
        strcpy(kstr, "NCKEY_DEL");
        break;
    case NCKEY_BACKSPACE:
        strcpy(kstr, "NCKEY_BACKSPACE");
        break;
    case NCKEY_PGDOWN:
        strcpy(kstr, "NCKEY_PGDOWN");
        break;
    case NCKEY_PGUP:
        strcpy(kstr, "NCKEY_PGUP");
        break;
    case NCKEY_HOME:
        strcpy(kstr, "NCKEY_HOME");
        break;
    case NCKEY_END:
        strcpy(kstr, "NCKEY_END");
        break;
    case NCKEY_F00:
        strcpy(kstr, "NCKEY_F00");
        break;
    case NCKEY_F01:
        strcpy(kstr, "NCKEY_F01");
        break;
    case NCKEY_F02:
        strcpy(kstr, "NCKEY_F02");
        break;
    case NCKEY_F03:
        strcpy(kstr, "NCKEY_F03");
        break;
    case NCKEY_F04:
        strcpy(kstr, "NCKEY_F04");
        break;
    case NCKEY_F05:
        strcpy(kstr, "NCKEY_F05");
        break;
    case NCKEY_F06:
        strcpy(kstr, "NCKEY_F06");
        break;
    case NCKEY_F07:
        strcpy(kstr, "NCKEY_F07");
        break;
    case NCKEY_F08:
        strcpy(kstr, "NCKEY_F08");
        break;
    case NCKEY_F09:
        strcpy(kstr, "NCKEY_F09");
        break;
    case NCKEY_F10:
        strcpy(kstr, "NCKEY_F10");
        break;
    case NCKEY_F11:
        strcpy(kstr, "NCKEY_F11");
        break;
    case NCKEY_F12:
        strcpy(kstr, "NCKEY_F12");
        break;
    case NCKEY_F13:
        strcpy(kstr, "NCKEY_F13");
        break;
    case NCKEY_F14:
        strcpy(kstr, "NCKEY_F14");
        break;
    case NCKEY_F15:
        strcpy(kstr, "NCKEY_F15");
        break;
    case NCKEY_F16:
        strcpy(kstr, "NCKEY_F16");
        break;
    case NCKEY_F17:
        strcpy(kstr, "NCKEY_F17");
        break;
    case NCKEY_F18:
        strcpy(kstr, "NCKEY_F18");
        break;
    case NCKEY_F19:
        strcpy(kstr, "NCKEY_F19");
        break;
    case NCKEY_F20:
        strcpy(kstr, "NCKEY_F20");
        break;
    case NCKEY_F21:
        strcpy(kstr, "NCKEY_F21");
        break;
    case NCKEY_F22:
        strcpy(kstr, "NCKEY_F22");
        break;
    case NCKEY_F23:
        strcpy(kstr, "NCKEY_F23");
        break;
    case NCKEY_F24:
        strcpy(kstr, "NCKEY_F24");
        break;
    case NCKEY_F25:
        strcpy(kstr, "NCKEY_F25");
        break;
    case NCKEY_F26:
        strcpy(kstr, "NCKEY_F26");
        break;
    case NCKEY_F27:
        strcpy(kstr, "NCKEY_F27");
        break;
    case NCKEY_F28:
        strcpy(kstr, "NCKEY_F28");
        break;
    case NCKEY_F29:
        strcpy(kstr, "NCKEY_F29");
        break;
    case NCKEY_F30:
        strcpy(kstr, "NCKEY_F30");
        break;
    case NCKEY_F31:
        strcpy(kstr, "NCKEY_F31");
        break;
    case NCKEY_F32:
        strcpy(kstr, "NCKEY_F32");
        break;
    case NCKEY_F33:
        strcpy(kstr, "NCKEY_F33");
        break;
    case NCKEY_F34:
        strcpy(kstr, "NCKEY_F34");
        break;
    case NCKEY_F35:
        strcpy(kstr, "NCKEY_F35");
        break;
    case NCKEY_F36:
        strcpy(kstr, "NCKEY_F36");
        break;
    case NCKEY_F37:
        strcpy(kstr, "NCKEY_F37");
        break;
    case NCKEY_F38:
        strcpy(kstr, "NCKEY_F38");
        break;
    case NCKEY_F39:
        strcpy(kstr, "NCKEY_F39");
        break;
    case NCKEY_F40:
        strcpy(kstr, "NCKEY_F40");
        break;
    case NCKEY_F41:
        strcpy(kstr, "NCKEY_F41");
        break;
    case NCKEY_F42:
        strcpy(kstr, "NCKEY_F42");
        break;
    case NCKEY_F43:
        strcpy(kstr, "NCKEY_F43");
        break;
    case NCKEY_F44:
        strcpy(kstr, "NCKEY_F44");
        break;
    case NCKEY_F45:
        strcpy(kstr, "NCKEY_F45");
        break;
    case NCKEY_F46:
        strcpy(kstr, "NCKEY_F46");
        break;
    case NCKEY_F47:
        strcpy(kstr, "NCKEY_F47");
        break;
    case NCKEY_F48:
        strcpy(kstr, "NCKEY_F48");
        break;
    case NCKEY_F49:
        strcpy(kstr, "NCKEY_F49");
        break;
    case NCKEY_F50:
        strcpy(kstr, "NCKEY_F50");
        break;
    case NCKEY_F51:
        strcpy(kstr, "NCKEY_F51");
        break;
    case NCKEY_F52:
        strcpy(kstr, "NCKEY_F52");
        break;
    case NCKEY_F53:
        strcpy(kstr, "NCKEY_F53");
        break;
    case NCKEY_F54:
        strcpy(kstr, "NCKEY_F54");
        break;
    case NCKEY_F55:
        strcpy(kstr, "NCKEY_F55");
        break;
    case NCKEY_F56:
        strcpy(kstr, "NCKEY_F56");
        break;
    case NCKEY_F57:
        strcpy(kstr, "NCKEY_F57");
        break;
    case NCKEY_F58:
        strcpy(kstr, "NCKEY_F58");
        break;
    case NCKEY_F59:
        strcpy(kstr, "NCKEY_F59");
        break;
    case NCKEY_F60:
        strcpy(kstr, "NCKEY_F60");
        break;
    case NCKEY_ENTER:
        strcpy(kstr, "NCKEY_ENTER");
        break;
    case NCKEY_CLS:
        strcpy(kstr, "NCKEY_CLS");
        break;
    case NCKEY_DLEFT:
        strcpy(kstr, "NCKEY_DLEFT");
        break;
    case NCKEY_DRIGHT:
        strcpy(kstr, "NCKEY_DRIGHT");
        break;
    case NCKEY_ULEFT:
        strcpy(kstr, "NCKEY_ULEFT");
        break;
    case NCKEY_URIGHT:
        strcpy(kstr, "NCKEY_URIGHT");
        break;
    case NCKEY_CENTER:
        strcpy(kstr, "NCKEY_CENTER");
        break;
    case NCKEY_BEGIN:
        strcpy(kstr, "NCKEY_BEGIN");
        break;
    case NCKEY_CANCEL:
        strcpy(kstr, "NCKEY_CANCEL");
        break;
    case NCKEY_CLOSE:
        strcpy(kstr, "NCKEY_CLOSE");
        break;
    case NCKEY_COMMAND:
        strcpy(kstr, "NCKEY_COMMAND");
        break;
    case NCKEY_COPY:
        strcpy(kstr, "NCKEY_COPY");
        break;
    case NCKEY_EXIT:
        strcpy(kstr, "NCKEY_EXIT");
        break;
    case NCKEY_PRINT:
        strcpy(kstr, "NCKEY_PRINT");
        break;
    case NCKEY_REFRESH:
        strcpy(kstr, "NCKEY_REFRESH");
        break;
    case NCKEY_SEPARATOR:
        strcpy(kstr, "NCKEY_SEPARATOR");
        break;
    case NCKEY_CAPS_LOCK:
        strcpy(kstr, "NCKEY_CAPS_LOCK");
        break;
    case NCKEY_SCROLL_LOCK:
        strcpy(kstr, "NCKEY_SCROLL_LOCK");
        break;
    case NCKEY_NUM_LOCK:
        strcpy(kstr, "NCKEY_NUM_LOCK");
        break;
    case NCKEY_PRINT_SCREEN:
        strcpy(kstr, "NCKEY_PRINT_SCREEN");
        break;
    case NCKEY_PAUSE:
        strcpy(kstr, "NCKEY_PAUSE");
        break;
    case NCKEY_MENU:
        strcpy(kstr, "NCKEY_MENU");
        break;
    case NCKEY_MEDIA_PLAY:
        strcpy(kstr, "NCKEY_MEDIA_PLAY");
        break;
    case NCKEY_MEDIA_PAUSE:
        strcpy(kstr, "NCKEY_MEDIA_PAUSE");
        break;
    case NCKEY_MEDIA_PPAUSE:
        strcpy(kstr, "NCKEY_MEDIA_PPAUSE");
        break;
    case NCKEY_MEDIA_REV:
        strcpy(kstr, "NCKEY_MEDIA_REV");
        break;
    case NCKEY_MEDIA_STOP:
        strcpy(kstr, "NCKEY_MEDIA_STOP");
        break;
    case NCKEY_MEDIA_FF:
        strcpy(kstr, "NCKEY_MEDIA_FF");
        break;
    case NCKEY_MEDIA_REWIND:
        strcpy(kstr, "NCKEY_MEDIA_REWIND");
        break;
    case NCKEY_MEDIA_NEXT:
        strcpy(kstr, "NCKEY_MEDIA_NEXT");
        break;
    case NCKEY_MEDIA_PREV:
        strcpy(kstr, "NCKEY_MEDIA_PREV");
        break;
    case NCKEY_MEDIA_RECORD:
        strcpy(kstr, "NCKEY_MEDIA_RECORD");
        break;
    case NCKEY_MEDIA_LVOL:
        strcpy(kstr, "NCKEY_MEDIA_LVOL");
        break;
    case NCKEY_MEDIA_RVOL:
        strcpy(kstr, "NCKEY_MEDIA_RVOL");
        break;
    case NCKEY_MEDIA_MUTE:
        strcpy(kstr, "NCKEY_MEDIA_MUTE");
        break;
    case NCKEY_LSHIFT:
        strcpy(kstr, "NCKEY_LSHIFT");
        break;
    case NCKEY_LCTRL:
        strcpy(kstr, "NCKEY_LCTRL");
        break;
    case NCKEY_LALT:
        strcpy(kstr, "NCKEY_LALT");
        break;
    case NCKEY_LSUPER:
        strcpy(kstr, "NCKEY_LSUPER");
        break;
    case NCKEY_LHYPER:
        strcpy(kstr, "NCKEY_LHYPER");
        break;
    case NCKEY_LMETA:
        strcpy(kstr, "NCKEY_LMETA");
        break;
    case NCKEY_RSHIFT:
        strcpy(kstr, "NCKEY_RSHIFT");
        break;
    case NCKEY_RCTRL:
        strcpy(kstr, "NCKEY_RCTRL");
        break;
    case NCKEY_RALT:
        strcpy(kstr, "NCKEY_RALT");
        break;
    case NCKEY_RSUPER:
        strcpy(kstr, "NCKEY_RSUPER");
        break;
    case NCKEY_RHYPER:
        strcpy(kstr, "NCKEY_RHYPER");
        break;
    case NCKEY_RMETA:
        strcpy(kstr, "NCKEY_RMETA");
        break;
    case NCKEY_L3SHIFT:
        strcpy(kstr, "NCKEY_L3SHIFT");
        break;
    case NCKEY_L5SHIFT:
        strcpy(kstr, "NCKEY_L5SHIFT");
        break;
    case NCKEY_MOTION:
        strcpy(kstr, "NCKEY_MOTION");
        break;
    case NCKEY_BUTTON1:
        strcpy(kstr, "NCKEY_BUTTON1");
        break;
    case NCKEY_BUTTON2:
        strcpy(kstr, "NCKEY_BUTTON2");
        break;
    case NCKEY_BUTTON3:
        strcpy(kstr, "NCKEY_BUTTON3");
        break;
    case NCKEY_BUTTON4:
        strcpy(kstr, "NCKEY_BUTTON4");
        break;
    case NCKEY_BUTTON5:
        strcpy(kstr, "NCKEY_BUTTON5");
        break;
    case NCKEY_BUTTON6:
        strcpy(kstr, "NCKEY_BUTTON6");
        break;
    case NCKEY_BUTTON7:
        strcpy(kstr, "NCKEY_BUTTON7");
        break;
    case NCKEY_BUTTON8:
        strcpy(kstr, "NCKEY_BUTTON8");
        break;
    case NCKEY_BUTTON9:
        strcpy(kstr, "NCKEY_BUTTON9");
        break;
    case NCKEY_BUTTON10:
        strcpy(kstr, "NCKEY_BUTTON10");
        break;
    case NCKEY_BUTTON11:
        strcpy(kstr, "NCKEY_BUTTON11");
        break;
    case NCKEY_SIGNAL:
        strcpy(kstr, "NCKEY_SIGNAL");
        break;
    case NCKEY_EOF:
        strcpy(kstr, "NCKEY_EOF");
        break;
    default:
        snprintf(kstr, 32, "NCKEY_%d", key_id);
        break;
    }
    return kstr;
}
