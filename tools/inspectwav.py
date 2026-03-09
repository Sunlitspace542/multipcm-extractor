import struct
import sys


def read_chunks(f):
    chunks = {}

    while True:
        header = f.read(8)
        if len(header) < 8:
            break

        chunk_id, chunk_size = struct.unpack("<4sI", header)
        chunk_id = chunk_id.decode("ascii", errors="replace")

        chunk_data = f.read(chunk_size)

        # chunks are padded to even sizes
        if chunk_size % 2 == 1:
            f.read(1)

        chunks[chunk_id] = chunk_data

    return chunks


def parse_fmt(data):
    fmt = struct.unpack("<HHIIHH", data[:16])

    return {
        "audio_format": fmt[0],
        "channels": fmt[1],
        "sample_rate": fmt[2],
        "byte_rate": fmt[3],
        "block_align": fmt[4],
        "bits_per_sample": fmt[5],
    }


def parse_smpl(data):
    header = struct.unpack("<9I", data[:36])

    smpl = {
        "manufacturer": header[0],
        "product": header[1],
        "sample_period": header[2],
        "midi_unity_note": header[3],
        "midi_pitch_fraction": header[4],
        "smpte_format": header[5],
        "smpte_offset": header[6],
        "num_sample_loops": header[7],
        "sampler_data": header[8],
        "loops": []
    }

    offset = 36

    for i in range(smpl["num_sample_loops"]):
        loop_struct = struct.unpack("<6I", data[offset:offset+24])

        loop = {
            "cue_point_id": loop_struct[0],
            "type": loop_struct[1],
            "start": loop_struct[2],
            "end": loop_struct[3],
            "fraction": loop_struct[4],
            "play_count": loop_struct[5]
        }

        smpl["loops"].append(loop)

        offset += 24

    return smpl


def inspect_wav(filename):
    with open(filename, "rb") as f:

        riff = f.read(12)
        riff_id, size, wave_id = struct.unpack("<4sI4s", riff)

        print("=== RIFF HEADER ===")
        print("ChunkID:", riff_id.decode())
        print("ChunkSize:", size)
        print("Format:", wave_id.decode())
        print()

        chunks = read_chunks(f)

        if "fmt " in chunks:
            print("=== FMT CHUNK ===")
            fmt = parse_fmt(chunks["fmt "])
            for k, v in fmt.items():
                print(f"{k}: {v}")
            print()
        else:
            print("No fmt chunk found\n")

        if "smpl" in chunks:
            print("=== SMPL CHUNK ===")
            smpl = parse_smpl(chunks["smpl"])

            for k, v in smpl.items():
                if k != "loops":
                    print(f"{k}: {v}")

            print()

            for i, loop in enumerate(smpl["loops"]):
                print(f"Loop {i}")
                for k, v in loop.items():
                    print(f"  {k}: {v}")
                print()

        else:
            print("No smpl chunk found")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python inspect_wav.py file.wav")
        sys.exit(1)

    inspect_wav(sys.argv[1])