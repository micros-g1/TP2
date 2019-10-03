#include "MK64F12.h"
#include <stdlib.h>
#include <SPI/spi_driver.h>

#define N_RXFIFO 4
#define N_TXFIFO 4

bool initialized = false;

enum{
	PA_SPI,
	PB_SPI,
	PC_SPI,
	PD_SPI,
	PE_SPI
};

typedef struct{
	int port;
	int pin;
	int alt;
}spi_pin_t;

typedef struct{
	bool cont_pcs;
	uint8_t ctar_n;
	bool eoq;
	bool cont_clear;
	uint8_t pcs_assert;
	uint8_t data;
}spi_push_data_t;

void spi_driver_mcr_init(void);
void spi_driver_port_init(void);
void spi_driver_setup_port(spi_pin_t pin_info);
void spi_driver_halt_module(bool value);
void spi_driver_ctar_init(void);
void spi_driver_push_txdata(uint8_t * data_in, int length);
bool spi_driver_is_running(void);
void spi_push_frame(spi_push_data_t push_data);


void spi_driver_init(void){
    if(!initialized){

        //Setup module pins.
        spi_driver_port_init();

        //Enbale Clock Gating
        SIM->SCGC6 |= SIM_SCGC6_SPI0(1);

        //Setup MCR to initial default state
        spi_driver_mcr_init();

        //Setup CTAR to initial default state
        spi_driver_ctar_init();

        initialized = true;
    }
}

/**
 * @brief SPI MCR register initialization
 * @details configures the SPI MCR register in master mode, using a default
 * set of configurations. Sets the pin mux for each SPI signal.
 */
void spi_driver_mcr_init(void){

	//DCONF. SPI
	SPI0->MCR &= ~SPI_MCR_DCONF(0b11);

    //MSTR. Master Mode
    SPI0->MCR |= SPI_MCR_MSTR(1);

    //PCSIS. PCSx active LOW.
    SPI0->MCR |= SPI_MCR_PCSIS(1);
    
    //MDIS. Disable module clock
    SPI0->MCR &= ~SPI_MCR_MDIS(1);

    //CLR_RXF. Flush RX FIFO & clear RX Counter
    SPI0->MCR |= SPI_MCR_CLR_RXF(1);

    //CLR_TXF. Flush TX FIFO & clear TX Counter
    SPI0->MCR |= SPI_MCR_CLR_TXF(1);

    //TODO: Check if FIFO are empty and redy to use.

    //CONT_SCKE. Continuous SCK enabled.
    SPI0->MCR &= ~SPI_MCR_CONT_SCKE(1);
}

/**
 * @brief SPI PORT configuration
 * @details sets the port configuration for each SPI signal. Using spi_driver_setup_port as
 * a helper.
 */
void spi_driver_port_init(void){

    //SPI0_PCS0:    PTC-4(Alt 2)
    //SPI0_SCK:     PTD-1(Alt 2)
    //SPI0_SOUT:    PTD-2(Alt 2)
    //SPI0_SIN:     PTD-3(Alt 2)

    spi_pin_t pcs0 	= {.port=PC_SPI, .pin=4, .alt=2};
    spi_pin_t sck	= {.port=PD_SPI, .pin=1, .alt=2};
    spi_pin_t sin	= {.port=PD_SPI, .pin=3, .alt=2};
    spi_pin_t sout	= {.port=PD_SPI, .pin=2, .alt=2};

    spi_driver_setup_port(pcs0);
    spi_driver_setup_port(sck);
    spi_driver_setup_port(sin);
    spi_driver_setup_port(sout);

}

/**
 * @brief Configures a specific pin PCR
 * @details sets the MUX and LK field for each pin PCR register.
 * @param port_ptr Pointer to the PORT_Type structure.
 * @param pin_info Structure containing the pin info.
 */
void spi_driver_setup_port(spi_pin_t pin_info){
	PORT_Type * port_ptr;
	switch(pin_info.port){
	case PA_SPI:
		port_ptr = PORTA;
		break;
	case PB_SPI:
		port_ptr = PORTB;
		break;
	case PC_SPI:
		port_ptr = PORTC;
		break;
	case PD_SPI:
		port_ptr = PORTD;
		break;
	case PE_SPI:
		port_ptr = PORTE;
		break;
	default:
		port_ptr = NULL;
		break;
	}
	if(port_ptr != NULL)
	{
		//Clear and set MUX field
		port_ptr->PCR[pin_info.pin] &= ~PORT_PCR_MUX_MASK;
		port_ptr->PCR[pin_info.pin] |= PORT_PCR_MUX(pin_info.alt);

		//Lock PIN.
		port_ptr->PCR[pin_info.pin] |= PORT_PCR_LK(1);
		//TODO: Veriicar que la configuracion de Lock sea compatible.
	}

}

/**
 * @brief Sets the HALT state of the module.
 * @param value boolean parameter that indicates the halt state to set.
 */
void spi_driver_halt_module(bool _value){
	if(_value)
		SPI0->MCR |= SPI_MCR_HALT(1);
	else
		SPI0->MCR &= ~SPI_MCR_HALT(1);
}

/**
 * @brief Configures the initial state of the CTAR register
 */
void spi_driver_ctar_init(void){

	//PCSSCK. PCS to SCK Prescaler value is 1.
	SPI0->CTAR[0] &= ~SPI_CTAR_PCSSCK(0b11);

	//FMSZ. Frame Size = 1Byte = 8bits.
	SPI0->CTAR[0] &= ~SPI_CTAR_FMSZ_MASK;
	SPI0->CTAR[0] |= SPI_CTAR_FMSZ(7);

	//BR.
	SPI0->CTAR[0] &= ~SPI_CTAR_BR_MASK;
	SPI0->CTAR[0] |= SPI_CTAR_BR(0b1111);

}

bool spi_driver_transfer_completed(void){
	return (SPI0->SR & SPI_SR_TCF_MASK) != 0;
}

bool spi_driver_is_running(void){
	return SPI0->SR & SPI_SR_TXRXS_MASK;
}

// Clock Configs CTAR

void spi_set_double_baud_rate(bool double_br){
	if(double_br)
		SPI0->CTAR[0] |= SPI_CTAR_DBR(1);
	else
		SPI0->CTAR[0] &= ~SPI_CTAR_DBR(1);
}

void spi_set_frame_size(uint8_t size){
	if(initialized){
		if(size > 3 && size <= N_TXFIFO){
				SPI0->CTAR[0] &= ~SPI_CTAR_FMSZ_MASK;
				SPI0->CTAR[0] |= SPI_CTAR_FMSZ(size-1);
			}
	}
}

void spi_set_clock_polarity(spi_cpol_t csk_pol){
	if(initialized){
		if(csk_pol == SPI_SCK_INACTIVE_LOW)
				SPI0->CTAR[0] &= ~SPI_CTAR_CPOL(1);
			else if(csk_pol == SPI_SCK_INACTIVE_HIGH)
				SPI0->CTAR[0] |= SPI_CTAR_CPOL(1);
	}
}

void spi_set_clock_phase(spi_cpha_t csk_phase){
	if(initialized){
		if(csk_phase == SPI_CPHA_CAP_IN_LEAD_CHANGE_FOLLOWING)
			SPI0->CTAR[0] &= ~SPI_CTAR_CPHA(1);
		else if(csk_phase == SPI_CPHA_CHANGE_IN_LEAD_CAP_FOLLOWING)
			SPI0->CTAR[0] |= SPI_CTAR_CPHA(1);
	}
}

void spi_set_transfer_order(spi_transfer_order_t order){
	if(initialized){
		if(order == SPI_LSB_FIRST)
			SPI0->CTAR[0] |= SPI_CTAR_LSBFE(1);
		else if(order == SPI_MSB_FIRST)
			SPI0->CTAR[0] &= ~SPI_CTAR_LSBFE(1);
	}
}

// Prescalers CTAR

void spi_set_pcs_to_sck_delay_prescaler(uint8_t delay_prescaler){
	if(initialized){
		if(delay_prescaler <= 3){
			SPI0->CTAR[0] &= ~SPI_CTAR_PCSSCK_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_PCSSCK(delay_prescaler);
		}
	}
}

void spi_set_after_sck_delay_prescaler(uint8_t delay_prescaler){
	if(initialized){
		if(delay_prescaler <= 3){
			SPI0->CTAR[0] &= ~SPI_CTAR_PASC_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_PASC(delay_prescaler);
		}
	}
}

void spi_set_after_transfer_prescaler(uint8_t delay_prescaler){
	if(initialized){
		if(delay_prescaler <= 3){
			SPI0->CTAR[0] &= ~SPI_CTAR_PDT_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_PDT(delay_prescaler);
		}
	}
}

void spi_set_baud_rate_prescaler(uint8_t br_prescaler){
	if(initialized){
		if(br_prescaler <= 3){
			SPI0->CTAR[0] &= ~SPI_CTAR_PBR_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_PBR(br_prescaler);
		}
	}
}

// Scalers

void spi_set_psc_to_sck_delay_scaler(uint8_t delay_scaler){
	if(initialized){
		if(delay_scaler < 16){
			SPI0->CTAR[0] &= ~SPI_CTAR_CSSCK_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_CSSCK(delay_scaler);
		}
	}
}

void spi_set_after_sck_delay_scaler(uint8_t delay_scaler){
	if(initialized){
		if(delay_scaler < 16){
			SPI0->CTAR[0] &= ~SPI_CTAR_ASC_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_ASC(delay_scaler);
		}
	}
}

void spi_set_after_transfer_delay_scaler(uint8_t delay_scaler){
	if(initialized){
		if(delay_scaler < 16){
			SPI0->CTAR[0] &= ~SPI_CTAR_DT_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_DT(delay_scaler);
		}
	}
}

void spi_set_baud_rate_scaler(uint8_t br_scaler){
	if(initialized){
		if(br_scaler < 16){
			SPI0->CTAR[0] &= ~SPI_CTAR_BR_MASK;
			SPI0->CTAR[0] |= SPI_CTAR_BR(br_scaler);
		}
	}
}

// push data to fifo
void spi_push_frame(spi_push_data_t push_data){
	if(initialized){
		// Solo pusheo si hay lugar en la fifo
		uint8_t tx_fifo_frames = (uint8_t)((SPI0->SR & SPI_SR_TXCTR_MASK) >> SPI_SR_TXCTR_SHIFT);
		uint32_t word = 0;
		word |= SPI_PUSHR_CONT_MASK;
		word |= SPI_PUSHR_PCS(1);
		if(tx_fifo_frames < N_TXFIFO){
			if(!push_data.cont_pcs)
				word &= ~SPI_PUSHR_CONT(1);
			word |= SPI_PUSHR_CTAS(0); // TODO: dejar que usuario elija CTARn??
			if(push_data.eoq)
				word |= SPI_PUSHR_EOQ(1);
			if(push_data.cont_clear)
				word |= SPI_PUSHR_CTCNT(1);
			word |= SPI_PUSHR_CONT(push_data.pcs_assert);
			word |= SPI_PUSHR_TXDATA(push_data.data);

			SPI0->PUSHR = word;
		}
	}
}

void spi_master_transfer_blocking(uint8_t * tx_data, uint8_t * rx_data, size_t length){
	uint32_t rx_index = 0;
	uint32_t tx_index = 0;
	bool push_finished = false; // true if all elements receibed in tx fifo
	bool pop_finished = false; // true if all elements taken from rx fifo

	// clear eoqf
	SPI0->SR |= SPI_SR_EOQF(1);

	//Clear FIFOs
	SPI0->MCR |= SPI_MCR_CLR_RXF(1);
	SPI0->MCR |= SPI_MCR_CLR_TXF(1);

	spi_push_data_t push_data;

	// if out not requested, consider pop finished
	if(rx_data == NULL)
		pop_finished = true;

	// while something to do...
	while(!push_finished || !pop_finished){
		// if pop finished and there is data to grab
		if(!pop_finished && (SPI0->SR & SPI_SR_RXCTR_MASK))
			rx_data[rx_index++] = (uint16_t)(SPI0->POPR & SPI_POPR_RXDATA_MASK);
			// if finished (i received all bytes)
		if(rx_index == length)
			pop_finished = true;

		// if push not finished, and out fifo not full,
		// then, if there is enough space in rx fifo or pop_finished
		// then send.
		// this will guarantee that we only send data to out queue if we know we have
		// enaugh space to store the corresponding data
		uint8_t frames_in_rx_fifo = (uint8_t)((SPI0->SR & SPI_SR_RXCTR_MASK) >> SPI_SR_RXCTR_SHIFT);
		uint8_t free_rx_fifo_frames = N_RXFIFO - frames_in_rx_fifo;
		uint8_t frames_in_tx_fifo = (uint8_t)((SPI0->SR & SPI_SR_TXCTR_MASK) >> SPI_SR_TXCTR_SHIFT);
		bool tx_fifo_full = (SPI0->SR & SPI_SR_TFFF_MASK) == 0;
		if(!push_finished && ! tx_fifo_full && (pop_finished || (frames_in_tx_fifo + 1 < free_rx_fifo_frames))){
			//push & transmit.

			push_data.eoq = (length == tx_index + 1);
			push_data.data = tx_data[tx_index++];
			push_data.cont_pcs = (tx_index < length);
			push_data.cont_clear = (tx_index == 0);
			spi_push_frame(push_data);

			spi_driver_halt_module(false);
			while(!(SPI0->SR & SPI_SR_TCF_MASK));
			spi_driver_halt_module(true);
			// clear tfff
			SPI0->SR |= SPI_SR_TFFF_MASK;

			// if all tx data ha been pushed, then push_completed
			if(tx_index == length)
				push_finished = true;
		}
	}
	//Clear
}
