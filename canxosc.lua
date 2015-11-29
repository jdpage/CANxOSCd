-- the CAN monitor device
device = {
    -- kind = "stn11xx",
    kind = "stdin",
    tty = "/dev/tty.OBDLinkMX-STN-SPP",
    transcript_file = "transcript.txt"
}

-- translation info
translation = {
    -- send the untranslated messages also?
    send_raw = true,

    -- send with a prefix
    prefix = "/mazda3",

    -- send to address
    address = "osc.udp://localhost:8000",

    -- use this map
    map = {
        [0x201] = function (rpm_hi, rpm_lo, _, _, speed_hi, speed_lo, accelerator)
            return {
                ["/rpm"] = { { "i", rpm_hi * 0x100 + rpm_lo } },
                ["/speed"] = { { "i", (speed_hi * 0x100 + speed_lo) / 100 } },
                ["/accelerator"] = { { "i", accelerator } },
            }
        end
    }
}
