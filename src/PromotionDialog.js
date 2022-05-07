import Modal from '@mui/material/Modal';
import Box from '@mui/material/Box';

function PieceImage(props) {
  const pieceName = props.color + " " + props.role;

  return (
    <piece className={pieceName + " promotion"}>
      <svg onClick={() => props.onClick(props.role)} viewBox="0 0 50 50" xmlns="http://www.w3.org/2000/svg">
        <use className={pieceName} href={'#' + props.role + '-svg'} />
      </svg>
    </piece>
  );
}

export default function PromotionDialog(props) {
  const style = {
    position: 'absolute',
    top: '50%',
    left: '50%',
    transform: 'translate(-50%, -50%)',
    width: 400,
    bgcolor: 'background.paper',
    border: '2px solid #000',
    boxShadow: 24,
    p: 4,
  };

  return (
    <Modal open={props.color !== undefined}>
      <Box sx={style}>
        <span>
          <PieceImage role="knight" color={props.color} onClick={props.onClick} />
          <PieceImage role="bishop" color={props.color} onClick={props.onClick} />
          <PieceImage role="rook" color={props.color} onClick={props.onClick} />
          <PieceImage role="queen" color={props.color} onClick={props.onClick} />
        </span>
      </Box>
    </Modal>
  );
}