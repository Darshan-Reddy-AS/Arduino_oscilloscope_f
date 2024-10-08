

void dataAnalize() {                       
  long d;
  long sum = 0;
  byte *waveBuff = data[sample+0];

  // search max and min value
  dataMin = 255;                          // min value initialize to big number
  dataMax = 0;                             // max value initialize to small number
  for (int i = 0; i < SAMPLES; i++) {     // serach max min value
    d = waveBuff[i];
    sum = sum + d;
    if (d < dataMin) {                     // update min
      dataMin = d;
    }
    if (d > dataMax) {                     // updata max
      dataMax = d;
    }
  }

  // calculate average
  dataAve = (10 * sum + (SAMPLES / 2)) / SAMPLES;  // Average value calculation (calculated by 10 times to improve accuracy)

}

void freqDuty() {                               // 周波数とデューティ比を求める detect frequency and duty cycle value from waveform data
  int swingCenter;                              // center of wave (half of p-p)
  float p0 = 0;                                 // 1-st posi edge
  float p1 = 0;                                 // total length of cycles
  float p2 = 0;                                 // total length of pulse high time
  float pFine = 0;                              // fine position (0-1.0)
  float lastPosiEdge;                           // last positive edge position

  float pPeriod;                                // pulse period
  float pWidth;                                 // pulse width

  int p1Count = 0;                              // wave cycle count
  int p2Count = 0;                              // High time count

  boolean a0Detected = false;
  //  boolean b0Detected = false;
  boolean posiSerch = true;                     // true when serching posi edge

  swingCenter = (3 * (dataMin + dataMax)) / 2;  // calculate wave center value

  for (int i = 1; i < SAMPLES - 2; i++) {      // scan all over the buffer
    if (posiSerch == true) {   // posi slope (frequency serch)
      if ((sum3(i) <= swingCenter) && (sum3(i + 1) > swingCenter)) {  // if across the center when rising (+-3data used to eliminate noize)
        pFine = (float)(swingCenter - sum3(i)) / ((swingCenter - sum3(i)) + (sum3(i + 1) - swingCenter) );  // fine cross point calc.
        if (a0Detected == false) {              // if 1-st cross
          a0Detected = true;                    // set find flag
          p0 = i + pFine;                       // save this position as startposition
        } else {
          p1 = i + pFine - p0;                  // record length (length of n*cycle time)
          p1Count++;
        }
        lastPosiEdge = i + pFine;               // record location for Pw calcration
        posiSerch = false;
      }
    } else {   // nega slope serch (duration serch)
      if ((sum3(i) >= swingCenter) && (sum3(i + 1) < swingCenter)) {  // if across the center when falling (+-3data used to eliminate noize)
        pFine = (float)(sum3(i) - swingCenter) / ((sum3(i) - swingCenter) + (swingCenter - sum3(i + 1)) );
        if (a0Detected == true) {
          p2 = p2 + (i + pFine - lastPosiEdge); // calucurate pulse width and accumurate it
          p2Count++;
        }
        posiSerch = true;
      }
    }
  }

  if (p1Count > 0 && p2Count > 0) {
    pPeriod = p1 / p1Count;                 // pulse period
    pWidth  = p2 / p2Count;                 // pulse width
  } else {
    pPeriod = 1.0e+37;  // set huge period to get 0Hz
    pWidth  = 0;        // pulse width
  }

  float fhref;
  if (rate > RATE_MAX) {    // Equivalent Time sampling
    fhref = ethref();
  } else {
    fhref = (float) pgm_read_dword(HREF + rate);
  }
  waveFreq = 10.0e6 / (fhref * pPeriod); // frequency
  waveDuty = 100.0 * pWidth / pPeriod;                                        // duty ratio
}

int sum3(int k) {       // Sum of before and after and own value
  byte *waveBuff = data[sample+0];
  int m = waveBuff[k - 1] + waveBuff[k] + waveBuff[k + 1];
  return m;
}

float ethref() {
  float href = 0.625; // micro second
  if (rate == RATE_MAX + 2) href *= 2.0;
  else if (rate == RATE_MAX + 3) href *= 5.0;
  else if (rate == RATE_MAX + 4) href *= 10.0;
  else if (rate == RATE_MAX + 5) href *= 20.0;
  else if (rate == RATE_MAX + 6) href *= 50.0;
  else if (rate == RATE_MAX + 7) href *= 100.0;
  return href;
}
