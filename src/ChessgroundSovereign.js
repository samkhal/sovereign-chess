import React from 'react'
import { Chessground as NativeChessground } from 'chessground-sovereign'
import "./assets/theme.css"
import "./assets/examples.css"
import "./assets/chessground.css"

export default class ChessgroundSovereign extends React.Component {
  componentDidMount() {
    this.cg = NativeChessground(this.el, this.props.config);
    // TODO(samkhal) why don't I need to forceUpdate here?
  }

  componentWillUnmount() {
    this.cg.destroy();
  }

  render() {
    if (this.cg)
      this.cg.set(this.props.config);

    return <div ref={el => this.el = el} />

  }

}