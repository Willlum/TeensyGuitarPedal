#ifndef control_aic23_h_
#define control_aic23_h_
 
#include "AudioControl.h"

/* Left (right) line input volume control register */
#define TLV320AIC23_LRS_ENABLED     0x0100
#define TLV320AIC23_LIM_MUTED       0x0080
#define TLV320AIC23_LIV_DEFAULT     0x0017
#define TLV320AIC23_LIV_MAX         0x001f
#define TLV320AIC23_LIV_MIN         0x0000

/* Left (right) channel headphone volume control register */
#define TLV320AIC23_LZC_ON      	 0x0080
#define TLV320AIC23_LHV_DEFAULT      0x0079
#define TLV320AIC23_LHV_MAX     	 0x007f
#define TLV320AIC23_LHV_MIN     	 0x0000

/* Analog audio path control register */
#define TLV320AIC23_STA_REG(x)      ((x)<<6)
#define TLV320AIC23_STE_ENABLED     0x0020
#define TLV320AIC23_DAC_SELECTED    0x0010
#define TLV320AIC23_BYPASS_ON       0x0008
#define TLV320AIC23_INSEL_MIC       0x0004
#define TLV320AIC23_MICM_MUTED      0x0002
#define TLV320AIC23_MICB_20DB       0x0001

/* Digital audio path control register */
#define TLV320AIC23_DACM_MUTE       0x0008
#define TLV320AIC23_DEEMP_32K       0x0002
#define TLV320AIC23_DEEMP_44K       0x0004
#define TLV320AIC23_DEEMP_48K       0x0006
#define TLV320AIC23_ADCHP_ON        0x0001

/* Power control down register */
#define DEVICE_PWR_OFF  0x0080	
#define CLK_OFF     	0x0040	
#define OSC_OFF		    0x0020	
#define OUT_OFF		    0x0010	
#define DAC_OFF    	    0x0008	
#define ADC_OFF    	    0x0004	
#define MIC_OFF    	    0x0002	
#define LINE_OFF      	0x0001	

/* Digital audio interface register */
#define TLV320AIC23_MS_MASTER       0x0040
#define TLV320AIC23_MS_SLAVE        0x0000
#define TLV320AIC23_LRSWAP_ON       0x0020
#define TLV320AIC23_LRP_ON      	0x0010
#define TLV320AIC23_IWL_16      	0x0000
#define TLV320AIC23_IWL_20      	0x0004
#define TLV320AIC23_IWL_24      	0x0008
#define TLV320AIC23_IWL_32          0x000C
#define TLV320AIC23_FOR_I2S     	0x0002
#define TLV320AIC23_FOR_DSP     	0x0003
#define TLV320AIC23_FOR_LJUST       0x0001

/* Sample rate control register */
#define TLV320AIC23_CLKOUT_HALF     0x0080
#define TLV320AIC23_CLKIN_HALF      0x0040
#define TLV320AIC23_BOSR_384fs      0x0002  /* BOSR_272fs in USB mode */
#define TLV320AIC23_USB_CLK_ON      0x0001
#define TLV320AIC23_SR_MASK         0xf
#define TLV320AIC23_CLKOUT_SHIFT    7
#define TLV320AIC23_CLKIN_SHIFT     6
#define TLV320AIC23_SR_SHIFT        2
#define TLV320AIC23_BOSR_SHIFT      1

/* Digital interface register */
#define DIGT_ACT_ON          0x1

#define TLV320AIC23_DEFAULT_OUT_VOL 0x70
#define TLV320AIC23_DEFAULT_IN_VOLUME   0x10

/* Control Registers TLV320AIC23 */
#define LINVOL_CTRL      	 0x00
#define RINVOL_CTRL      	 0x01
#define LCHNVOL_CTRL     	 0x02
#define RCHNVOL_CTRL     	 0x03
#define ANLG_CTRL        	 0x04
#define DIGT_CTRL        	 0x05
#define POWER_CTRL        	 0x06
#define DIGT_FMT_CTRL        0x07
#define SRATE_CTRL       	 0x08
#define DIGT_ACT_CTRL  	     0x09
#define RESET_CTRL       	 0x0F

#define I2C_ADDRESS	         0x1A
#define DEFAULT_VOL_CTRL     0x117
#define DEFAULT_ANLG_CTRL	 0x12
#define DEFAULT_DIGT_CTRL	 0x0

class AudioControlAIC23 : public AudioControl
{
	 public:
		bool enable(void);
		bool disable(void){return true;}
		bool volume(float volume){return true;} // volume 0.0 to 1.0
		bool inputLevel(float volume);  // volume 0.0 to 1.0
		bool inputSelect(int n){return true;}
        void mute(bool softMute);
		void bypass(bool bp);
	private:
		unsigned int read(unsigned int reg);		
		bool write(unsigned int reg, unsigned int val);
		void aic23_init(void);
		void power(int pdc);
		void reset(void);
		void digitalAudioFormat(void);
		void digitalAudioPathControl(void);
		void sampleRateControl(void); // Default setting is correct
 };

 
 

#endif /* control_aic23_h_ */