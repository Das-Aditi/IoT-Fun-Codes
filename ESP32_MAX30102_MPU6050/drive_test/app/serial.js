// Web Serial fallback module for direct browser-to-ESP32 testing
const SerialService = {
  port: null,
  reader: null,

  async connect() {
    if ("serial" in navigator) {
      try {
        this.port = await navigator.serial.requestPort();
        await this.port.open({ baudRate: 115200 });
        this.readStream();
      } catch (err) {
        console.warn("Serial connection skipped:", err);
      }
    }
  },

  async readStream() {
    const textDecoder = new TextDecoderStream();
    this.port.readable.pipeTo(textDecoder.writable);
    this.reader = textDecoder.readable.getReader();
    while (true) {
      const { value, done } = await this.reader.read();
      if (done) break;
      if (value) console.log("[Serial Raw]:", value);
    }
  }
};