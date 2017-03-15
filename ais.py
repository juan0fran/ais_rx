#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Ais
# Generated: Wed Mar 15 10:01:38 2017
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt4 import Qt
from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import qtgui
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
import kiss
import math
import sip
import sys


class ais(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Ais")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Ais")
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "ais")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())

        ##################################################
        # Variables
        ##################################################
        self.source_rate = source_rate = 1e6
        self.samp_rate = samp_rate = 48000
        
        self.variable_low_pass_filter_taps_0 = variable_low_pass_filter_taps_0 = firdes.low_pass(1.0, source_rate, 16e3, 1e3, firdes.WIN_HAMMING, 6.76)
          
        self.sps = sps = 5
        self.radtohertz = radtohertz = (samp_rate*2.0)/(2.0*math.pi)
        self.offset = offset = 0
        self.chan2_deviation = chan2_deviation = 25e3
        self.chan1_deviation = chan1_deviation = -25e3

        ##################################################
        # Blocks
        ##################################################
        self._offset_range = Range(-5e3, 5e3, 100, 0, 200)
        self._offset_win = RangeWidget(self._offset_range, self.set_offset, "Offset", "counter_slider", float)
        self.top_layout.addWidget(self._offset_win)
        self.rational_resampler_xxx_0_0 = filter.rational_resampler_ccc(
                interpolation=int(samp_rate) * 2,
                decimation=int(source_rate),
                taps=None,
                fractional_bw=None,
        )
        self.rational_resampler_xxx_0 = filter.rational_resampler_ccc(
                interpolation=int(samp_rate) * 2,
                decimation=int(source_rate),
                taps=None,
                fractional_bw=None,
        )
        self.qtgui_sink_x_0 = qtgui.sink_f(
        	4096, #fftsize
        	firdes.WIN_BLACKMAN_hARRIS, #wintype
        	0, #fc
        	samp_rate*2, #bw
        	"", #name
        	True, #plotfreq
        	True, #plotwaterfall
        	True, #plottime
        	True, #plotconst
        )
        self.qtgui_sink_x_0.set_update_time(1.0/0.1)
        self._qtgui_sink_x_0_win = sip.wrapinstance(self.qtgui_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_sink_x_0_win)
        
        self.qtgui_sink_x_0.enable_rf_freq(False)
        
        
          
        self.qtgui_number_sink_0 = qtgui.number_sink(
            gr.sizeof_float,
            0.6,
            qtgui.NUM_GRAPH_HORIZ,
            2
        )
        self.qtgui_number_sink_0.set_update_time(0.10)
        self.qtgui_number_sink_0.set_title("")
        
        labels = ["Frequency (Upper)", "Frequency (Lower)", "Error", "", "",
                  "", "", "", "", ""]
        units = ["Hz", "Hz", "Hz", "", "",
                 "", "", "", "", ""]
        colors = [("blue", "red"), ("blue", "red"), ("black", "black"), ("black", "black"), ("black", "black"),
                  ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black")]
        factor = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        for i in xrange(2):
            self.qtgui_number_sink_0.set_min(i, -5e3)
            self.qtgui_number_sink_0.set_max(i, 5e3)
            self.qtgui_number_sink_0.set_color(i, colors[i][0], colors[i][1])
            if len(labels[i]) == 0:
                self.qtgui_number_sink_0.set_label(i, "Data {0}".format(i))
            else:
                self.qtgui_number_sink_0.set_label(i, labels[i])
            self.qtgui_number_sink_0.set_unit(i, units[i])
            self.qtgui_number_sink_0.set_factor(i, factor[i])
        
        self.qtgui_number_sink_0.enable_autoscale(False)
        self._qtgui_number_sink_0_win = sip.wrapinstance(self.qtgui_number_sink_0.pyqwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_number_sink_0_win)
        self.low_pass_filter_0_0 = filter.fir_filter_ccf(2, firdes.low_pass(
        	1, samp_rate * 2, 6e3, 1e3, firdes.WIN_HAMMING, 6.76))
        self.low_pass_filter_0 = filter.fir_filter_ccf(2, firdes.low_pass(
        	1, samp_rate * 2, 6e3, 1e3, firdes.WIN_HAMMING, 6.76))
        self.kiss_pdu_to_kiss_0 = kiss.pdu_to_kiss()
        self.kiss_nrzi_decode_0_0 = kiss.nrzi_decode()
        self.kiss_nrzi_decode_0 = kiss.nrzi_decode()
        self.kiss_hdlc_deframer_0_0 = kiss.hdlc_deframer(check_fcs=True, max_length=10000)
        self.kiss_hdlc_deframer_0 = kiss.hdlc_deframer(check_fcs=True, max_length=10000)
        self.freq_xlating_fir_filter_xxx_0_0 = filter.freq_xlating_fir_filter_ccc(1, (variable_low_pass_filter_taps_0), chan2_deviation, source_rate)
        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(1, (variable_low_pass_filter_taps_0), chan1_deviation + offset, source_rate)
        self.digital_gfsk_demod_0_0 = digital.gfsk_demod(
        	samples_per_symbol=5,
        	sensitivity=0.5,
        	gain_mu=0.175,
        	mu=0.5,
        	omega_relative_limit=0.005,
        	freq_error=0.0,
        	verbose=False,
        	log=False,
        )
        self.digital_gfsk_demod_0 = digital.gfsk_demod(
        	samples_per_symbol=5,
        	sensitivity=0.5,
        	gain_mu=0.175,
        	mu=0.5,
        	omega_relative_limit=0.005,
        	freq_error=0.0,
        	verbose=False,
        	log=False,
        )
        self.digital_fll_band_edge_cc_0_0 = digital.fll_band_edge_cc(10, 0.35, 25, 0.05)
        self.digital_fll_band_edge_cc_0 = digital.fll_band_edge_cc(10, 0.35, 25, 0.05)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, source_rate,True)
        self.blocks_socket_pdu_0 = blocks.socket_pdu("TCP_SERVER", "", "51999", 10000, False)
        self.blocks_null_sink_1 = blocks.null_sink(gr.sizeof_char*1)
        self.blocks_null_sink_0_0 = blocks.null_sink(gr.sizeof_float*1)
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_float*1)
        self.blocks_multiply_xx_0 = blocks.multiply_vcc(1)
        self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_vff((radtohertz, ))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vff((radtohertz, ))
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_gr_complex*1, "/home/gs-ms/AIS_RX/Raw-FirstTestPort", True)
        self.blocks_complex_to_real_1 = blocks.complex_to_real(1)
        self.analog_sig_source_x_0 = analog.sig_source_c(samp_rate*2, analog.GR_COS_WAVE, 20e3, 1, 0)

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.kiss_hdlc_deframer_0, 'out'), (self.kiss_pdu_to_kiss_0, 'in'))    
        self.msg_connect((self.kiss_hdlc_deframer_0_0, 'out'), (self.kiss_pdu_to_kiss_0, 'in'))    
        self.msg_connect((self.kiss_pdu_to_kiss_0, 'out'), (self.blocks_socket_pdu_0, 'pdus'))    
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_multiply_xx_0, 0))    
        self.connect((self.blocks_complex_to_real_1, 0), (self.qtgui_sink_x_0, 0))    
        self.connect((self.blocks_file_source_0, 0), (self.blocks_throttle_0, 0))    
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.qtgui_number_sink_0, 0))    
        self.connect((self.blocks_multiply_const_vxx_0_0, 0), (self.qtgui_number_sink_0, 1))    
        self.connect((self.blocks_multiply_xx_0, 0), (self.blocks_complex_to_real_1, 0))    
        self.connect((self.blocks_throttle_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))    
        self.connect((self.blocks_throttle_0, 0), (self.freq_xlating_fir_filter_xxx_0_0, 0))    
        self.connect((self.digital_fll_band_edge_cc_0, 1), (self.blocks_multiply_const_vxx_0, 0))    
        self.connect((self.digital_fll_band_edge_cc_0, 3), (self.blocks_null_sink_0, 1))    
        self.connect((self.digital_fll_band_edge_cc_0, 2), (self.blocks_null_sink_0, 0))    
        self.connect((self.digital_fll_band_edge_cc_0, 0), (self.low_pass_filter_0_0, 0))    
        self.connect((self.digital_fll_band_edge_cc_0_0, 1), (self.blocks_multiply_const_vxx_0_0, 0))    
        self.connect((self.digital_fll_band_edge_cc_0_0, 3), (self.blocks_null_sink_0_0, 1))    
        self.connect((self.digital_fll_band_edge_cc_0_0, 2), (self.blocks_null_sink_0_0, 0))    
        self.connect((self.digital_fll_band_edge_cc_0_0, 0), (self.low_pass_filter_0, 0))    
        self.connect((self.digital_gfsk_demod_0, 0), (self.kiss_nrzi_decode_0, 0))    
        self.connect((self.digital_gfsk_demod_0_0, 0), (self.kiss_nrzi_decode_0_0, 0))    
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.rational_resampler_xxx_0, 0))    
        self.connect((self.freq_xlating_fir_filter_xxx_0_0, 0), (self.rational_resampler_xxx_0_0, 0))    
        self.connect((self.kiss_nrzi_decode_0, 0), (self.kiss_hdlc_deframer_0_0, 0))    
        self.connect((self.kiss_nrzi_decode_0_0, 0), (self.blocks_null_sink_1, 0))    
        self.connect((self.kiss_nrzi_decode_0_0, 0), (self.kiss_hdlc_deframer_0, 0))    
        self.connect((self.low_pass_filter_0, 0), (self.digital_gfsk_demod_0, 0))    
        self.connect((self.low_pass_filter_0_0, 0), (self.digital_gfsk_demod_0_0, 0))    
        self.connect((self.rational_resampler_xxx_0, 0), (self.blocks_multiply_xx_0, 1))    
        self.connect((self.rational_resampler_xxx_0, 0), (self.digital_fll_band_edge_cc_0_0, 0))    
        self.connect((self.rational_resampler_xxx_0_0, 0), (self.digital_fll_band_edge_cc_0, 0))    

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "ais")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()


    def get_source_rate(self):
        return self.source_rate

    def set_source_rate(self, source_rate):
        self.source_rate = source_rate
        self.blocks_throttle_0.set_sample_rate(self.source_rate)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_radtohertz((self.samp_rate*2.0)/(2.0*math.pi))
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate*2)
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate * 2, 6e3, 1e3, firdes.WIN_HAMMING, 6.76))
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate * 2, 6e3, 1e3, firdes.WIN_HAMMING, 6.76))
        self.qtgui_sink_x_0.set_frequency_range(0, self.samp_rate*2)

    def get_variable_low_pass_filter_taps_0(self):
        return self.variable_low_pass_filter_taps_0

    def set_variable_low_pass_filter_taps_0(self, variable_low_pass_filter_taps_0):
        self.variable_low_pass_filter_taps_0 = variable_low_pass_filter_taps_0
        self.freq_xlating_fir_filter_xxx_0.set_taps((self.variable_low_pass_filter_taps_0))
        self.freq_xlating_fir_filter_xxx_0_0.set_taps((self.variable_low_pass_filter_taps_0))

    def get_sps(self):
        return self.sps

    def set_sps(self, sps):
        self.sps = sps

    def get_radtohertz(self):
        return self.radtohertz

    def set_radtohertz(self, radtohertz):
        self.radtohertz = radtohertz
        self.blocks_multiply_const_vxx_0.set_k((self.radtohertz, ))
        self.blocks_multiply_const_vxx_0_0.set_k((self.radtohertz, ))

    def get_offset(self):
        return self.offset

    def set_offset(self, offset):
        self.offset = offset
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.chan1_deviation + self.offset)

    def get_chan2_deviation(self):
        return self.chan2_deviation

    def set_chan2_deviation(self, chan2_deviation):
        self.chan2_deviation = chan2_deviation
        self.freq_xlating_fir_filter_xxx_0_0.set_center_freq(self.chan2_deviation)

    def get_chan1_deviation(self):
        return self.chan1_deviation

    def set_chan1_deviation(self, chan1_deviation):
        self.chan1_deviation = chan1_deviation
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.chan1_deviation + self.offset)


def main(top_block_cls=ais, options=None):

    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
